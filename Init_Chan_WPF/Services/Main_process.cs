using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using System.Runtime.InteropServices;

namespace Init_Chan_WPF.Services
{
    internal class Main_process
    {


    
        // 导入 DLL 中的函数
        [DllImport("./cppDll.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern void StartMonitoring(string processName);

        [DllImport("./cppDll.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern void StopMonitoring();

        [DllImport("./cppDll.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern void TerminateProcessByName(string processName);
        [DllImport("./cppDll.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern void SetMouseAcceleration(bool mouseAccel);

        public static void stdprocess(string processName)
        {
            string processNames = processName;
            // 启动监控
            //StartMonitoring(processNames);


            Console.WriteLine("Monitoring started. Press Enter to stop...");
            Console.ReadLine();
            SetMouseAcceleration(false);
            // 停止监控
            StopMonitoring();
            Console.WriteLine("Monitoring stopped.");

        }

}
}
