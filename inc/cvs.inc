<?php
/**
 * Basic Class for working with CVS-Repositories
 */
class CVS {
	/**
	 * root-directory of CVS
	 *
	 * @var string
	 */
	protected $_cvsroot;

	/**
	 * Configuration
	 *
	 * @var array
	 */
	protected $_cfg;

	/**
	 * Constructor
	 *
	 * @param [type] $cfg
	 */
	public function __construct(&$cfg)
	{
		$this->_cfg =& $cfg;
	}

	/**
	 * Initialize the CVS Root Direcotry
	 *
	 * @param string $cvsroot
	 * 
	 * @return void
	 */
	public function init($cvsroot) 
	{
		$this->_cvsroot = $cvsroot;
		putenv("CVSROOT={$cvsroot}");
	}

	/**
	 * Provide Diff of File and given Revision
	 *
	 * @param string $file
	 * @param string $rev
	 * @return void
	 */
	public function get_diff($file, $rev)
	{
		$results = "";

		if (!file_exists($this->_cfg['tools']['cvs']) || !is_executable($this->_cfg['tools']['cvs']))
			return false;

		if (!file_exists($file) || !is_readable($file))
			return false;

		$cmd  = "{$this->_cfg['tools']['cvs']} diff {$this->_cfg['diff']['params']} ";
		$cmd .= "-r {$rev['old']} -r {$rev['new']} {$file}";

		if (!($cvscmd = popen($cmd, "r")))
			return false;

		$format = strtolower($this->_cfg['diff']['output_format']);

		while (!feof($cvscmd)) {
			$line = fgets($cvscmd, 256);
			$line = str_replace($this->_cfg['paths']['base'], "", $line);

			if ($format != "fulldiff") {
				$pfx = substr($line, 0, 1);

				if (($pfx != "+") && ($pfx != "-"))
					continue;
			}

			if ($format == "english") {
				if (substr($line, 0, 3) == "+++") {
					$line = str_replace("+++", "New Revision", $line);
				} else if (substr($line, 0, 3) == "---") {
					$line = str_replace("---", "Old Revision", $line);
				} else {
					$line = str_replace("+", "Add ", $line);
					$line = str_replace("-", "Remove ", $line);
				}
			}

			$results .= $line;
		}

		fclose($cvscmd);

		return $results;
	}

	/**
	 * Return complete file from CVS Repository
	 *
	 * @param string $file
	 * @return void
	 */
	public function get_complete($file)
	{
		$results = "";

		if (!($routes = fopen($file, "r")))
			return false;

		while (!feof($routes))
			$results .= fgets($routes, 256);

		fclose($routes);

		return $results;
	}

	/**
	 * Updating a File
	 *
	 * @param string $file
	 * @return void
	 */
	public function update($file)
	{

		if (!file_exists($this->_cfg['tools']['cvs']) || !is_executable($this->_cfg['tools']['cvs']))
			return false;

		chdir($this->_cfg['paths']['db']);

		/* CVS add */
		if (!($results = popen("{$cfg['tools']['cvs']} add ${file} 2>&1", "r")))
			return false;

		while (!feof($results)) {
			$line = rtrim(fgets($results, 256));

			if (strstr($line, "already exists")) {
				$tmp = explode(" ", $line);
				$rev['old'] = $tmp[8];
				$rev['new'] = $tmp[8];
			}
		}

		pclose($results);

		/* CVS commit */
		if (!($results = popen("{$this->_cfg['tools']['cvs']} commit -m update {$file}", "r")))
			return false;

		while (!feof($results)) {
			$line = rtrim(fgets($results, 256));

			if (strstr($line, "initial revision")) {
				$tmp = explode(":", $line);
				$rev['old'] = "0.00";
				$rev['new'] = trim($tmp[1]);

				return $rev;
			}

			if (strstr($line, "new revision")) {
				$tmp = explode(":", $line);
				$rev['old'] = trim($tmp[2]);
				$tmp = explode(";", $tmp[1]);
				$rev['new'] = trim($tmp[0]);

				return $rev;
			}
		}

		pclose($results);

		return $rev;
	}
}

?>