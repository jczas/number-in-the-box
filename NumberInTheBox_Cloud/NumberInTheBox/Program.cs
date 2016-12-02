using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using Microsoft.Azure.Devices.Client;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;
using System.Net;

namespace NumberInTheBox
{
    class Program
    {
        static string CLIENT_ID;
        static byte isConnected = 0;
        static MqttClient client;
        static void Main(string[] args)
        {
            client = new MqttClient("test.mosquitto.org");
            client.ConnectionClosed += Client_ConnectionClosed;
            CLIENT_ID = Guid.NewGuid().ToString();
            Connect();
            SendMessages();
        }
        private static async Task<ParkingInfo> GetParkingInfo()
        {
            using (var client = new HttpClient())
            {
                string repUrl = "https://mysterious-coast-1799.herokuapp.com/freeParkplaces";
                HttpResponseMessage response = await client.GetAsync(repUrl);
                if (response.IsSuccessStatusCode)
                {
                    string result = await response.Content.ReadAsStringAsync();
                    var rootResult = JsonConvert.DeserializeObject<ParkingInfo>(result);
                    return rootResult;
                }
                else
                {
                    return null;
                }
            }
        }

        private static void Connect()
        {
            isConnected = client.Connect(CLIENT_ID);
        }

        private static void SendMessages()
        {
            int oldFreeParkPlaces = -1;
            while (true && isConnected == MqttMsgConnack.CONN_ACCEPTED)
            {
                var parkingInfo = GetParkingInfo().Result;
                if (oldFreeParkPlaces != parkingInfo.freeParkplaces)
                {
                    string strValue = Convert.ToString(parkingInfo.freeParkplaces);
                    client.Publish("/number-in-the-box", Encoding.UTF8.GetBytes(strValue), 1, true);
                    oldFreeParkPlaces = parkingInfo.freeParkplaces;
                }
                Thread.Sleep(1000);
            }

        }

        private static void Client_ConnectionClosed(object sender, EventArgs e)
        {
            isConnected = MqttMsgConnack.CONN_REFUSED_IDENT_REJECTED;
            Connect();
            SendMessages();
        }
    }
}
