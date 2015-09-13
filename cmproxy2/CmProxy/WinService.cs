namespace CmProxy
{
    using System;
    using System.IO;
    using System.Net.Sockets;
    using System.Collections.Generic;
    using System.Threading;
    using System.Net;
    using System.Text;
    using System.Diagnostics;
    using System.Text.RegularExpressions;
    using System.Security.Cryptography;
    using System.Collections;

    class WinService : System.ServiceProcess.ServiceBase
    {

        private TcpListener tcpListener;
        private Thread listenThread;

        private DateTime procOutputDateTime;
        Process proc;
        Thread procTimeoutThread;

        private DateTime playingDateTime;
        private int playingPeriod;
        private int playingPriority;
        private String playingCommand;

        private bool busy;

        public static void TestRunFromConsole()
        {
            bool isValidProperties = ReadPropertiesFile();

            if (isValidProperties)
            {
                WinService win = new WinService();
                win.InitializeComponent();
                win.OnStart(null);
            }
        }

        
        // The main entry point for the process
        public static void Main()
        {
            bool isValidProperties = ReadPropertiesFile();

            if (isValidProperties)
            {
                System.ServiceProcess.ServiceBase[] ServicesToRun;
                ServicesToRun =
                  new System.ServiceProcess.ServiceBase[] { new WinService() };
                System.ServiceProcess.ServiceBase.Run(ServicesToRun);

            }
        }
        
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.ServiceName = Constant.SERVICE_SERVICE_NAME;
        }

        /// <summary>
        /// Set things in motion so your service can do its work.
        /// </summary>
        protected override void OnStart(string[] args)
        {
            this.playingDateTime = DateTime.Now;
            this.playingPeriod = 0;
            this.playingPriority = 0;
            this.playingCommand = "";

            this.busy = false;

            this.tcpListener = new TcpListener(IPAddress.Any, Int16.Parse(Constant.CONFIG_LISTEN_PORT));

            // Don't allow another socket to bind to this port.
            //this.tcpListener.Server.ExclusiveAddressUse = false;

            // The socket will linger for 10 seconds after  
            // Socket.Close is called.
            //this.tcpListener.Server.LingerState = new LingerOption(true, 10);

            // Disable the Nagle Algorithm for this tcp socket.
            //this.tcpListener.Server.NoDelay = true;

            // Set the timeout for synchronous receive methods to  
            // 1 second (1000 milliseconds.)
            //this.tcpListener.Server.ReceiveTimeout = 5000;

            // Set the timeout for synchronous send methods 
            // to 1 second (1000 milliseconds.)			
            //this.tcpListener.Server.SendTimeout = 5000;

            this.listenThread = new Thread(new ThreadStart(ListenForClients));
            try
            {
                this.listenThread.Start();
            }
            catch (ThreadStartException) { log2("##### Failed to start thread"); }
            log2("Service started version: " + Constant.CONFIG_VERSION + ", listen on:" + Constant.CONFIG_LISTEN_PORT);

        }
        /// <summary>
        /// Stop this service.
        /// </summary>
        protected override void OnStop()
        {
            if (this.tcpListener != null) this.tcpListener.Stop();
            log2("Service stopped");
        }
        // --------------------------------------------------------
        // --------------------------------------------------------
        // --------------------------------------------------------

        private static bool ReadPropertiesFile()
        {

            bool isValidProperties = true;

            if (System.IO.File.Exists(Constant.PATH_CONFIG_FILE))
            {
                // read from config
                //load
                Constant.config = new Properties(Constant.PATH_CONFIG_FILE);

                // check required parameters
                if (!Constant.config.ContainsKey("version")
                    || !Constant.config.ContainsKey("debug")
                    || !Constant.config.ContainsKey("listen_port")
                    || !Constant.config.ContainsKey("trigger_ip")
                    || !Constant.config.ContainsKey("trigger_port")
                    || !Constant.config.ContainsKey("command_separator")
                    || !Constant.config.ContainsKey("command_length")
                    ) isValidProperties = false;

            }
            if (isValidProperties)
            {
                //get value whith default value
                Constant.CONFIG_VERSION = Constant.config.get("version");
                Constant.CONFIG_LISTEN_PORT = Constant.config.get("listen_port");

                Constant.CONFIG_COMMAND_SEPARATOR = Constant.config.get("command_separator");
                Constant.CONFIG_COMMAND_LENGTH = Int32.Parse(Constant.config.get("command_length"));
            }
            else
            {
                log("cannot find CmProxy.properties file");
            }

            return isValidProperties;
        }

        private void ListenForClients()
        {

            this.tcpListener.Start();

            while (true)
            {

                try
                {
                    //blocks until a client has connected to the server
                    TcpClient client = this.tcpListener.AcceptTcpClient();

                    //create a thread to handle communication 
                    //with connected client
                    Thread clientThread = new Thread(new ParameterizedThreadStart(HandleClientComm));
                    try
                    {
                        clientThread.Start(client);
                    }
                    catch (ThreadStartException) { log2("##### Failed to start thread 3"); }
                }
                catch (Exception e)
                {
                    if (this.tcpListener != null) this.tcpListener.Stop();
                    log("Error ListenForClients:" + e.Message);
                }
            }

        }
        private void HandleClientComm(object client)
        {
            try
            {
                TcpClient tcpClient = (TcpClient)client;
                NetworkStream clientStream = tcpClient.GetStream();

                // refresh properties file
                if (!ReadPropertiesFile())
                {
                    //tcpClient.Close();
                    log("error propoties");
                    return;
                }

                byte[] message = new byte[4096];
                int bytesRead = 0;

                // read until client disconnect
                while (true)
                {
                    try
                    {
                        //blocks until a client sends a message
                        bytesRead = clientStream.Read(message, 0, 4096);
                    }
                    catch
                    {
                        //a socket error has occured
                        break;
                    }

                    if (bytesRead == 0)
                    {
                        //the client has disconnected from the server
                        break;
                    }

                    //message has successfully been received
                    ASCIIEncoding encoder = new ASCIIEncoding();
                    // String commandString = encoder.GetString(message, 0, bytesRead);

                    // -----------------------------------------------------------------------
                    // -----------------------------------------------------------------------
                    // -----------------------------------------------------------------------
                    // -----------------------------------------------------------------------



                    ArrayList array = getMessageArray(encoder.GetString(message, 0, bytesRead));

                    try
                    {
                        for (int i = 0; i < array.Count; i++)
                        {

                            //String commandString = array[i].ToString();
                            String[] tmp = (array[i].ToString()).Split('#');
                            if (tmp == null || tmp.Length != 3) continue;

                            String command = tmp[0];
                            String period = tmp[1];
                            String priority = tmp[2];

                            log("-----------------------------------");
                            log("Cmd array: " + array[i].ToString());
                            log("Check TriggerCM: " + command);

                            String response = "";

                            if (this.busy)
                            {
                                log("Busy !!! ignore command..." + command);
                                response = "1000";

                            }
                            else if (CheckPriority(priority))
                            {
                                log("Interupt by priority " + priority);

                                // interrupt by priority
                                Thread exeThread = new Thread(new ParameterizedThreadStart(TriggerCM));
                                try
                                {
                                    exeThread.Start(array[i].ToString());
                                }
                                catch (ThreadStartException) { log2("##### Failed to start thread 4"); }
                                int timeRemainToExecuteNextCommand = CheckTimeRemain();
                                response = timeRemainToExecuteNextCommand.ToString();
                            }
                            else
                            {

                                // check time
                                int timeRemainToExecuteNextCommand = CheckTimeRemain();
                                if (timeRemainToExecuteNextCommand == 0)
                                {
                                    Thread exeThread = new Thread(new ParameterizedThreadStart(TriggerCM));
                                    try
                                    {
                                        exeThread.Start(array[i].ToString());
                                    }
                                    catch (ThreadStartException) { log2("##### Failed to start thread 5 "); }
                                    timeRemainToExecuteNextCommand = CheckTimeRemain();
                                }
                                else
                                {
                                    log("Command ignore: " + command + ", Time Remain:" + timeRemainToExecuteNextCommand + " ms");
                                }
                                response = timeRemainToExecuteNextCommand.ToString();

                            }
                        }

                    }
                    catch (Exception e)
                    {
                        log("Unexpected Error" + e.StackTrace);
                    }

                    // -----------------------------------------------------------------------
                    // -----------------------------------------------------------------------
                    // -----------------------------------------------------------------------
                    // -----------------------------------------------------------------------


                } // end while
                //clientStream.Close();
                //tcpClient.Close();
                //log("Closed connection");
            }
            catch (Exception)
            {
            }
        }
        // --------------------------------------------------------
        // --------------------------------------------------------
        // --------------------------------------------------------

        private bool CheckPriority(String priority)
        {

            try
            {
                if (Int32.Parse(priority) > this.playingPriority || Int32.Parse(priority) == 0)
                {
                    return true;
                }
            }
            catch (Exception)
            {
                return false;
            }

            return false;
        }

        // return time remain to execute next command (ms)
        private int CheckTimeRemain()
        {
            try
            {
                DateTime now = DateTime.Now;
                double timeFromLastestStart = (now - playingDateTime).TotalSeconds;

                //log("CheckTimeRemain: " + timeFromLastestStart.ToString() +
                //    "," + playingPeriod
                //    );

                if (timeFromLastestStart > playingPeriod) return 0;

                return (int)(((playingPeriod * 1.0) - timeFromLastestStart) * 1000);
            }
            catch (Exception) { return 0; }
        }

        // call trigger
        private void TriggerCM(object cmdString)
        {
            try
            {
                String[] tmp = (cmdString.ToString()).Split('#');
                if (tmp == null || tmp.Length != 3) return;

                String command = tmp[0];
                String period = tmp[1];
                String priority = tmp[2];

                // Prepare the process to run
                ProcessStartInfo processInfo = new ProcessStartInfo();
                processInfo.ErrorDialog = false;
                processInfo.UseShellExecute = false;
                processInfo.RedirectStandardOutput = true;

                // Enter in the command line arguments, everything you would enter after the executable name itself
                processInfo.Arguments = "/H:"
                                    + Constant.config.get("trigger_ip")
                                    + " /P:"
                                    + Constant.config.get("trigger_port")
                                    + " /T:"
                                    + command.ToString()
                                    + " /V:CM2x";

                // Enter the executable to run, including the complete path
                log("PATH_TRIGGER_EXE:" + Constant.PATH_TRIGGER_EXE);
                processInfo.FileName = Constant.PATH_TRIGGER_EXE;

                // Do you want to show a console window?
                if (Constant.config.get("debug") == "true")
                {
                    processInfo.WindowStyle = ProcessWindowStyle.Normal;
                }
                else
                {
                    processInfo.WindowStyle = ProcessWindowStyle.Hidden;
                }
                processInfo.CreateNoWindow = true;
                log(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
                log(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
                log(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
                log(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
                //log2(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
                log2(">>>>> " + processInfo.FileName + " " + processInfo.Arguments);

                this.busy = true;

                // Run the external process & wait for it to finish
                this.proc = Process.Start(processInfo);

                procOutputDateTime = DateTime.Now;

                proc.OutputDataReceived += (sender, outputLine) =>
                {
                    if (outputLine.Data != null)
                    {
                        log(">>>>> " + outputLine.Data + "," + sender.ToString());
                        if (this.procTimeoutThread != null && this.procTimeoutThread.IsAlive) this.procTimeoutThread.Abort();
                        this.procTimeoutThread = (new Thread(new ThreadStart(KillProcessWhenTimeout)));
                        try
                        {
                            this.procTimeoutThread.Start();
                        }
                        catch (ThreadStartException) { log2("##### Failed to start thread 0"); }


                    }
                };
                proc.BeginOutputReadLine();

                try
                {
                    proc.WaitForExit();
                }
                catch (Exception ex)
                {
                    log("Exception while process cmd: " + ex.Message);
                }
                // Retrieve the app's exit code
                log(">>>>> Exit code: " + proc.ExitCode);

                if (proc.ExitCode == 0)
                {
                    this.playingDateTime = DateTime.Now;
                    this.playingPeriod = Int32.Parse(period);
                    this.playingPriority = Int32.Parse(priority);
                    this.playingCommand = command;
                }


                this.busy = false;
            }
            catch (Exception ex) { log("TriggerCM : " + ex.Message); }
        }

        private void KillProcessWhenTimeout()
        {
            Thread.Sleep(Constant.PROCESS_IDLE_TIMEOUT);
            log(">>>>> KillProcessTimeout: " + Constant.PROCESS_IDLE_TIMEOUT);
            this.busy = false;
            try
            {
                if (this.proc != null) this.proc.Kill();
            }
            catch (Exception) { }
        }
        // --------------------------------------------------------
        // --------------------------------------------------------
        // --------------------------------------------------------

        // --------------------------------------------------------
        // --------------------------------------------------------
        // --------------------------------------------------------
        // utils

        public static void log(String msg)
        {
            /*
            String folder = Constant.PATH_LOG_FOLDER + "\\" + DateTime.Now.ToString("yyyyMM", System.Globalization.CultureInfo.InvariantCulture);
            String logfilename = "log_" + DateTime.Now.ToString("yyyyMMddHH", System.Globalization.CultureInfo.InvariantCulture) + ".txt";

            if (!System.IO.Directory.Exists(folder))
                System.IO.Directory.CreateDirectory(folder);

            FileStream fs = new FileStream(folder + "\\" + logfilename,
                              FileMode.OpenOrCreate, FileAccess.Write);
            StreamWriter m_streamWriter = new StreamWriter(fs);
            m_streamWriter.BaseStream.Seek(0, SeekOrigin.End);
            m_streamWriter.WriteLine("[" + DateTime.Now.ToShortDateString() + " " + DateTime.Now.ToLongTimeString() + "] "
                + msg
                + "\n");
            m_streamWriter.Flush();
            m_streamWriter.Close();
            */
            //log2(msg);
        }

        public static void log2(String msg)
        {
            //Console.WriteLine(msg);
            
            try
            {
                String folder = Constant.PATH_LOG_FOLDER + "\\" + DateTime.Now.ToString("yyyyMM", System.Globalization.CultureInfo.InvariantCulture);
                String logfilename = "log_" + DateTime.Now.ToString("yyyyMMddHH", System.Globalization.CultureInfo.InvariantCulture) + ".txt";

                if (!System.IO.Directory.Exists(folder))
                    System.IO.Directory.CreateDirectory(folder);

                FileStream fs = new FileStream(folder + "\\" + logfilename,
                                  FileMode.OpenOrCreate, FileAccess.Write);
                StreamWriter m_streamWriter = new StreamWriter(fs);
                m_streamWriter.BaseStream.Seek(0, SeekOrigin.End);
                m_streamWriter.WriteLine("[" + DateTime.Now.ToShortDateString() + " " + DateTime.Now.ToLongTimeString() + "] "
                    + msg
                    + "\n");
                m_streamWriter.Flush();
                m_streamWriter.Close();
            }
            catch (Exception) { }
             
        }

        // --------------------------------------------------------
        // --------------------------------------------------------
        // --------------------------------------------------------

        object oLock = new object();

        private String bufferMessage = "";

        private ArrayList getMessageArray(String msg)
        {

            ArrayList array = new ArrayList();

            lock (oLock)
            {
                try
                {
                    if (bufferMessage.Length > 1024) bufferMessage = bufferMessage.Substring(bufferMessage.Length - 1024, 1024);
                    bufferMessage += msg;

                    int index = -1;

                    do
                    {
                        index = bufferMessage.IndexOf(Constant.CONFIG_COMMAND_SEPARATOR);
                        if (index == -1) break;

                        if (bufferMessage.Length >= index + Constant.CONFIG_COMMAND_SEPARATOR.Length + Constant.CONFIG_COMMAND_LENGTH)
                        {

                            String command = bufferMessage.Substring(index + Constant.CONFIG_COMMAND_SEPARATOR.Length, Constant.CONFIG_COMMAND_LENGTH);
                            bufferMessage = bufferMessage.Substring(index + Constant.CONFIG_COMMAND_SEPARATOR.Length + Constant.CONFIG_COMMAND_LENGTH);

                            array.Add(command);
                            //Console.WriteLine("command : " + command);
                        }
                        else
                        {
                            break;
                        }
                    } while (index != -1);
                }
                catch (Exception) { }
            }
            return array;
        }

        // --------------------------------------------------------
        // --------------------------------------------------------
        // --------------------------------------------------------
    }
}