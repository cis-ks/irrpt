<?php
/**
 * Class to create a temporary file and this will be deleted on destruction of the class
 */
class TempFile
{
    /**
     * Path and Filename of the temporary File
     *
     * @var string
     */
    public $path;

    /**
     * Constructuor
     *
     * @param string $suffix
     */
    public function __construct($suffix = 'tempfile', $directory = sys_get_temp_dir())
    {
        $this->path = tempnam($directory, $suffix);
    }

    /**
     * Destructor - deleting file on exit.
     */
    public function __destruct()
    {
        unlink($this->path);
    }
}
?>