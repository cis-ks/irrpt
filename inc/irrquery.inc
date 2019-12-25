<?php
/* Requiring necessary includes */
require_once(rtrim(__DIR__, '/') . '/status.inc');

/**
 * Generic class to do raw IRR-Queries
 */
class IRRQuery
{
    /**
     * Hostname for Querying data
     * @var string $hostname
     */
    protected $_hostname = '';

    /**
     * Port for IRR-Server
     * @var integer $port
     */
    protected $_port = 43;

    /**
     * Socket for Communication with IRR
     * @var filedescriptor $communication
     */
    protected $_communication;

    /**
     * Switch to use Caching
     * @var boolean $caching
     */
    protected $_caching = false;

    /**
     * Internal Cache
     * @var array $cache
     */
    protected $_cache;

    /**
     * Last command send
     * @var string $lastcommand
     */
    public $lastcommand;

    /**
     * Constructor Functions
     * 
     * @return 
     */
    public function __construct($host = null, $port = null) {
        if($host != null && $port != null) {
            return $this->connect($host, $port);
        }
    }

    protected function _multi_mode() {
        $this->_send("!!\n");
        return true;
    }

    /**
     * Identify our softwaretool to the IRR
     *
     * @return boolean
     */
    protected function _identify() {
        $this->_send("!nIRRPowerTools2\n");
        $this->_response();

        return true;
    }

    /**
     * Sending data to the IRR-Server
     *
     * @param string $output
     * @return boolean
     */
    protected function _send($output) {
        $this->lastcommand = rtrim($output);

        if (fwrite($this->_communication, $output) == false) {
            status(STATUS_ERROR, "IRR Query - Unexpected write() error on socket.");
            return false;
        }
    
        return true;
    }

    /**
     * Read one line of buffer
     *
     * @return string
     */
    protected function _readline() {
        if (($data = fgets($this->_communication, 256)) == false) {
            status(STATUS_ERROR, "IRR Query - Unexpected read() error on socket.");
            return false;
        }
  
        return $data;
    }
  
    /**
     * Processing Response
     *
     * @return string
     */
    function _response() {
        $data = "";
        $datalen = 0;

        $header = rtrim($this->_readline());

        switch ($header[0]) {
            case "C":
                return true;
            case "D":
                status(STATUS_WARNING, "IRR Query - key not found - Query: " . $this->lastcommand);
                return false;
            case "E":
                return true;
            case "F":
                status(STATUS_WARNING, "Query ({$this->lastcommand}) failed:" . substr($header, 1));
                return false;
        }

        $header = rtrim($header);

        if ($header[0] == 'A') {
            $datalen = intval(substr($header,1));
        }
        else {
            status(STATUS_ERROR, "IRR Query - Parse error looking for data length.");
            return false;
        }

        while (strlen($data) < $datalen)
            $data .= $this->_readline();

        if ($datalen != strlen($data))
            status(STATUS_ERROR, "IRR Query - Data read doesn't match expected length.");

        $footer = $this->_readline();

        return rtrim($data);
    }
  
    /*
    * Not sure why this is a good idea, but IRR still lets people register
    * classful routes with no /prefixlength. Scan for this and fix it.
    */

    /**
     * Not sure why this is a good idea, but IRR still lets people register
     * classful routes with no /prefixlength. Scan for this and fix it.
     *
     * @param string $routes
     * @return string
     */
    protected function _classful_fix($routes) {
        for ($i=0;$i<sizeof($routes);$i++) {
            if (strrpos($routes[$i], "/") == false) {
                $octets = explode(".", $routes[$i]);
                $first = intval($octets[0]);

                if (($first) > 0 && ($first < 128)) {
                    $length = "/8";
                }
                elseif (($first) > 0 && ($first < 192)) {
                    $length = "/16";
                }
                elseif (($first) > 0 && ($first < 255)) {
                    $length = "/24";
                }
                else {
                    status(STATUS_ERROR, "Invalid prefix: {$routes[$i]}");
                    unset($routes[$i]);
                }

                $routes[$i] .= $length;
            }
        }

        return $routes;
    }
  
    /**
     * Connect to Server
     *
     * @param string $host
     * @param integer $port
     * @return void
     */
    public function connect($host, $port) {
        $this->_hostname = $host;
        $this->_port = $port;

        @$this->_communication = fsockopen($this->_hostname, $this->_port, $errno, $errstr, 15);

        if (!$this->_communication) {
            status(STATUS_ERROR, "{$errstr} (errno {$errno})");
            return false;
        }

        $this->_multi_mode();
        $this->_identify();

        return $this;
    }
  
    /**
     * Closing the Connection to the Server
     *
     * @return void
     */
    public function close() {
        $this->_send("!q\n");

        fclose($this->_communication);
    }
  
    /**
     * Set the used sources
     *
     * @param string $sources
     * @return void
     */
    public function set_sources($sources)
    {
        $this->_send("!s-{$sources}\n");

        if (($results = $this->_response()) == false)
            return false;

        return explode(" ", $results);
    }
  
    /**
     * Get the uses Sources
     *
     * @return void
     */
    public function get_sources()
    {
        $this->_send("!s-lc\n");

        if (($results = $this->_response()) == false)
            return false;

        return $results;
    }
  
    /**
     * Setting the Cach-Mode
     *
     * @param boolean $mode
     * 
     * @return void
     */
    public function set_cache($mode)
    {
        if(is_bool($mode)) {
            $this->_caching = $mode;
        }
        else {
            $this->_caching = false;
        }
    }
  
    /**
     * Clearing the Cache
     *
     * @return void
     */
    public function clear_cache()
    {
        unset($this->cache);
    }

    /**
     * Query the Cache
     *
     * @param string $type
     * @param string $record
     * 
     * @return void
     */
    protected function _query_cache($type, $record)
    {
        if ($this->_caching) {
            if (isset($this->_cache[$type][$record])) {
                /* printf("Cache hit: %s\n", $record); */
                $this->_cache[$type][$record]['hits']++;

                return $this->_cache[$type][$record]['data'];
            }
        }

        return false;
    }

    /**
     * Adding Data to Cache
     *
     * @param string $type
     * @param string $record
     * @param string $data
     * 
     * @return void
     */  
    protected function _add_cache($type, $record, $data)
    {
        $this->_cache[$type][$record]['data'] = $data;
        $this->_cache[$type][$record]['time'] = time();
        $this->_cache[$type][$record]['hits'] = 0;
    }

    /**
     * Getting All Routes by Specific Origin-AS
     *
     * @param string $origin
     * @param integer $version
     * 
     * @return void
     */
    public function get_routes_by_origin($origin, $version)
    {
        if(!in_array($version, [4, 6]))
            return false;

        $type = ($version == 6) ? 'origin6' : 'origin';
        $cmd = ($version == 6) ? '6' : 'g';

        /* Prepend 'AS' to set if not provided */
        if (preg_match("/^AS./i", $origin) == 0) 
            $origin = 'AS'.$origin;
  
        /* Anything cached? */
        if (($results = $this->_query_cache($type, $origin)) != false)
            return $results;
  
        /* Get v4 prefixes */
        $this->_send("!{$cmd}{$origin}\n");

        if (($results = $this->_response()) == false) {
            return false;
        }
        else  {
            $results = ($version == 4) ? $this->_classful_fix(explode(" ", $results)) : explode(" ", $results);

            $results = ($version == 6) ? array_map('strtolower', $results) : $results;
            natsort($results);
            $results = array_values(array_unique($results));

            if ($this->_caching)
                $this->_add_cache("origin", $origin, $results);
        }
        return $results;
    }

    /**
     * Get all AS-Members by AS-Set
     *
     * @param string $set
     * @param boolean $recursive
     * 
     * @return array
     */
    public function get_members_by_set($set, $recursive = false)
    {
        /* Prepend 'AS' to set if not provided */
        if (preg_match("/^AS./i", $set) == 0) 
            $set = 'AS'.$set;

        /* Query the set data */
        if ($recursive == true) {
            $this->_send("!i{$set},1\n");
        }
        else {
            $this->_send("!i{$set}\n");
        }       

        if (($response = $this->_response()) == false) {
            return false;
        }

        $data = explode(" ", $response);
        $data = array_values(array_unique($data));
        return $data;
    }

    /**
     * Getting all Data by an AS-Set
     *
     * @param string $set
     * @param integer $version
     * @param boolean $splitbyorigin
     * 
     * @return array
     */
    public function get_data_by_set($set, $version, $splitbyorigin = false)
    {
        /* Defining the result-Array */
        $routes4 = [];
        $routes6 = [];

        $results4 = [];
        $results6 = [];

        $stats['missing_autnum_v4']  = 0;
        $stats['missing_autnum_v6']  = 0;

        /* Prepend 'AS' to set if not provided */
        if (preg_match("/^AS./i", $set) == 0) 
            $set = 'AS'.$set;
  
        /* Getting the Autonomous List from Set */
        $autnumlist = $this->get_members_by_set($set, true);
        if($autnumlist === false)
            return false;

        /* Detect route-set, otherwise assume as-set or autnum */
        if ( strstr($autnumlist[0], '.') != false && stristr($autnumlist[0], 'AS') == false && $version == 4) {
            $routes4[0] = $this->_classful_fix($autnumlist);
        }
        else {
            foreach($autnumlist as $asnumber) {
                $results = $this->get_routes_by_origin($asnumber, $version);
                if($results == false) {
                    $stats['missing_autnum_v' . $version]++;
                    continue;
                }

                if($splitbyorigin == true) {
                    if(array_key_exists(strtoupper($asnumber), ${'results'.$version})) {
                        ${'results'.$version}[$asnumber] = array_merge(${'results'.$version}[$asnumber], $results);
                    }
                    else {
                        ${'results'.$version}[$asnumber] = $results;
                    }
                }
                else {
                    ${'results'.$version}[0] = (array_key_exists(0, ${'results'.$version})) ? array_merge(${'results'.$version}[0], $results) : $results;
                }
            }
        }

        foreach([4, 6] as $v)
        {
            foreach(${'results'.$v} as $idx => $routes) {
                natsort($routes);
                $results['routes'.$v][$idx] = array_values(array_unique($routes));
            }
        }

        $results['asns'] = array_values(array_unique($autnumlist));
        $results['stats'] = $stats;

        return $results;
    }

    /**
     * Resolving Data by Object and Version into Array
     *
     * @param string $object
     * @param integer $version
     * @param boolean $splitbyorigin
     * 
     * @return array
     */
    public function resolve($object, $version, $splitbyorigin = false)
    {
        $results['routes'] = $this->get_routes_by_origin($object, $version);
        if($results['routes'] == false) {
            if( ($data = $this->get_data_by_set($object, $version, $splitbyorigin)) != false) {
                $results['routes'] = $data['routes'.$version];
                $results['asnlist'] = $data['asns'];
            }
        }
        else {
            $results['asnlist'][0] = $object;
        }

        if($results['routes'] == false) {
            status(STATUS_WARNING, "No registered v".$version." routes found for this AS.");
        }
        
        return $results;
    }
}
?>