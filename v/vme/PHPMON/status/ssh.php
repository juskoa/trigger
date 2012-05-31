<?php

/**
 * SSH2
 * 
 * @package Pork
 * @author SchizoDuckie
 * @version 1.0
 * @access public
 */
class SSH2
{
        private $host;
        private $port;
        private $connection;
        private $timeout;
        private $debugMode;
        private $debugPointer;
        public $connected; 
        public $error;


        /**
         * SSH2::__construct()
         * 
         * @param mixed $host
         * @param integer $port
         * @param integer $timeout
         * @return
         */
        function __construct($host, $port=22, $timeout=10)
        {
                $this->host = $host;
                $this->port = $port;
                $this->timeout = 10;
                $this->error = 'not connected';
                $this->connection = false;
                $this->debugMode = Settings::Load()->->get('Debug', 'Debugmode');
                $this->debugPointer = ($this->debugMode) ? fopen('./logs/'.date('Y-m-d--H-i-s').'.log', 'w+') : false;
                $this->connected = false;

        }


        /**
         * SSH2::connect()
         * 
         * @param mixed $username
         * @param mixed $password
         * @return
         */
        function connect($username, $password)
        {
                $this->connection = ssh2_connect($this->host, $this->port);
                if (!$this->connection) return $this->error("Could not connect to {$this->host}:{$this->port}");
                $this->debug("Connected to {$this->host}:{$this->port}");
                $authenticated = ssh2_auth_password($this->connection, $username, $password);
                if(!$authenticated) return $this->error("Could not authenticate: {$username}, check your password");
                $this->debug("Authenticated successfully as {$username}");
                $this->connected = true;

                return true;
        }

        /**
         * SSH2::exec()
         *
         * @param mixed $command shell command to execute
         * @param bool $onAvailableFunction a function to handle any available data.
         * @param bool $blocking blocking or non-blocking mode. This 'hangs' php execution until the command has completed if you set it to true. If you just want to start an import and go on, use this icm onAvailableFunction and false
         * @return
         */
        function exec($command, $onAvailableFunction=false, $blocking=true)
        {
                $output = '';
                $stream = ssh2_exec($this->connection, $command);
                $this->debug("Exec: {$command}");
                if($onAvailableFunction !== false)
                {
                        $lastReceived = time();
                        $timeout =false;
                        while (!feof($stream) && !$timeout)
                        {
                                $input = fgets($stream, 1024);
                                if(strlen($input) >0)
                                {
                                        call_user_func($onAvailableFunction, $input);
                                        $this->debug($input);
                                        $lastReceived = time();
                                }
                                else
                                {
                                        if(time() - $lastReceived >= $this->timeout)
                                        {
                                                $timeout = true;
                                                $this->error('Connection timed out');
                                                return($this->error);
                                        }
                                }
                        }
                }
                if($blocking === true && $onAvailableFunction === false)
                {
                        stream_set_blocking($stream, true);
                        $output = stream_get_contents($stream);
                        $this->debug($output);
                }
                fclose($stream);
                return($output);
        }


        /**
         * SSH2::createDirectory()
         *
         * Creates a directory via sftp
         *
         * @param string $dirname
         * @return boolean success
         *      
         */
        function createDirectory($dirname)
        {
                $ftpconnection = ssh2_sftp ($this->connection);
                $dircreated = ssh2_sftp_mkdir($ftpconnection, $dirname, true);
                if(!$dircreated) 
                {
                        $this->debug("Directory not created: ".$dirname);
                }
                return $dircreated;
        }

        public function listFiles($dirname)
        {
                $input = $this->exec(escapeshellcmd("ls  {$dirname}"));
                return(explode("\n", trim($input)));

        }
        public function sendFile($filename, $remotename)
        {
                $this->debug("sending {$filename} to {$remotename} ");
                if(file_exists($filename) && is_readable($filename))
                {
                        $result = ssh2_scp_send($this->connection, $filename, $remotename, 0664);
                }
                else
                {
                        $this->debug("Unable to read file : ".$filename);
                        return false;
                }
                if(!$result) $this->debug("Failure uploading {$filename} to {$remotename}");
                return $result;
        }
        public function getFile($remotename, $localfile)
        {
                $this->debug("grabbing {$remotename} to {$localfile}");
                $result = ssh2_scp_recv($this->connection, $remotename, $localfile);

                if(!$result) $this->debug("Failure downloading {$remotename} to {$localfile}");
                return $result;
        }

        /**
         * SSH2::debug()
         * 
         * @param mixed $message
         * @return
         */
        function debug($message) 
        {
                if($this->debugMode)
                {
                        fwrite($this->debugPointer, date('Y-m-d H:i:s')." : ".$message."\n");
                }
        }



        /**
         * SSH2::error()
         * 
         * @param mixed $errorMsg
         * @return
         */
        function error($errorMsg) 
        {
                $this->error = $errorMsg;
                $this->debug($errorMsg);
                return false;
        }
        /**
         * SSH2::__destruct()
         * 
         * @return
         */
        function __destruct() 
        {
                if($this->connection){
                        $this->connection = null;
                }
                if($this->debugMode && $this->debugPointer)
                {
                        fclose($this->debugPointer);
                }
        }

}
?>