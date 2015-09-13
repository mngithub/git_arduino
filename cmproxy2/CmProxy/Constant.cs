using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace CmProxy
{
    class Constant
    {
        public static String SERVICE_DISPLAY_NAME   = "cmproxy2";
        public static String SERVICE_SERVICE_NAME   = "cmproxy2";
        public static String SERVICE_DESCRIPTION    = "Listen for request and then trigger CM player.";

        public static String PATH_LOG_FOLDER        = @"C:\cmproxy2\log";
        public static String PATH_CONFIG_FILE       = @"C:\cmproxy2\service\CmProxy.properties";
        public static String PATH_TRIGGER_EXE        = @"""C:\cmproxy2\bin\Ryarc.TriggerControlConsole.exe""";

        // kill process if no any output
        // ms
        public static int PROCESS_IDLE_TIMEOUT      = 5000;

        public static Properties config;

        // read from config
        public static String CONFIG_VERSION;
        public static String CONFIG_LISTEN_PORT;
        
        public static String CONFIG_COMMAND_SEPARATOR;
        public static Int32 CONFIG_COMMAND_LENGTH;

    }
}
