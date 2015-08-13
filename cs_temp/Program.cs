using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace cs_temp
{
	class Program
	{
		public static string Client_IP
		{
			get
			{
				IPHostEntry host = Dns.GetHostEntry(Dns.GetHostName());
				string ClientIP = string.Empty;
				for (int i = 0; i < host.AddressList.Length; i++) {
				//	if (host.AddressList[i].AddressFamily == AddressFamily.InterNetwork) {
						ClientIP += "\n" + host.AddressList[i].ToString();
				//	}
				}
				return ClientIP;
			}
		}
		static void Main(string[] args)
		{
			System.Console.WriteLine(Client_IP);
		}
	}
}
