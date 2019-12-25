<?php
/* Including required Utils */
require_once(rtrim(__DIR__, '/') . '/status.inc');
require_once(rtrim(__DIR__, '/') . '/ipv6.inc');
require_once(rtrim(__DIR__, '/') . '/utils.inc');

class irrpt
{
    public static function track(&$cvs, $file)
    {  
        if (($rev = $cvs->update($file)) == FALSE) {
            status(STATUS_ERROR, "Error: Unable to perform CVS update, aborting.");
            exit(1);
        }
    
        if ($rev['old'] == "0.00")
            status(STATUS_INFO, sprintf("   - Importing %-45s version %s", $file, $rev['new']));
        else if ($rev['old'] != $rev['new'])
            status(STATUS_INFO, sprintf("   - Updating  %-45s version %s -> %s", $file, $rev['old'], $rev['new']));
    
        return $rev;
    }

    public static function update_email(&$cvs, &$cfg, $file, $email, $asString, $object, $rev, $type)
    {
        $content = "";
    
        if ($rev['old'] == $rev['new']) {
            return;
        }
    
        $headers  = "From: {$cfg['update']['from']}\n";
        $headers .= "Reply-To: {$cfg['update']['reply-to']}\n";
        $headers .= "Date: " . date("r") . "\n";
    
        if ($rev['old'] != "0.00") {
            $content .= "Changes for {$asString} (object {$object}) ({$type}):\n\n";
            $content .= $cvs->get_diff($file, $rev);
            $subject = $cfg['update']['subject'] . "{$asString} ({$type} Changes)";
        } else {
            $subject = $cfg['update']['subject'] . "{$asString} ({$type} Initial Import)";
        }
    
        $content .= "Complete list for {$asString} (object {$object}) ({$type}):\n";
        $content .= $cvs->get_complete($file);
        
        if (!($email == "-" || $email == "none" || !isset($email))) {
            status(STATUS_INFO, "   - Sending update notification to {$email}");
            mail($email, $subject, $content, $headers);
        }
    
        if (isset($cfg['update']['localcopy']))
            mail($cfg['update']['localcopy'], $subject, $content, $headers);
    }

    public static function process(&$cfg, &$irr, $peeras, $version, $object)
    {
        $resolve_result = $irr->resolve($object, $version, $cfg['fetch']['splitbyorigin']);

        status(STATUS_NOTICE, "Received " . count($resolve_result['routes']) . " AS-Numbers for {$object}");

        /* Strip out excluded routes that can not be registered */
        status(STATUS_NOTICE, "Filtering against excluded routes.");

        foreach($resolve_result['routes'] as $asnumber => $routesAs)
        {
            $intasnumber = preg_replace('/^AS/', '', $asnumber);
            $routefile = null;

            if($cfg['fetch']['splitbyorigin'] == false && $intasnumber == 0)
            {
                $routefile = $cfg['paths']['db'] . $peeras . "." . $version;
                utils::check_file_perms($routefile);

            } elseif($intasnumber != 0) {
                $routefile = $cfg['paths']['db'] . $intasnumber . "." . $version;
                utils::check_file_perms($routefile);                
            }

            if($routefile != null)
            {
                $aggfile = $routefile . $cfg['aggregate']['suffix'];
                utils::check_file_perms($aggfile);

                $routes = exclusions::filter($cfg['cfgfiles']['exclusions'], $routesAs);

                /* Write the routes we've just looked up into a file */
                status(STATUS_NOTICE, "Writing routes to local database.");

                if (($output = fopen($routefile, 'w')) == false)
                {
                    status(STATUS_WARNING, "Can not open IRRDB output file for {$asn}, skipping this record.");
                }

                if(is_array($routesAs))
                {
                    foreach($routesAs as $route)
                    {
                        fwrite($output, sprintf("%s".PHP_EOL, $route));
                    }
                }

                fclose($output);
                utils::update_file_perms($routefile, $cfg['cfgfiles']['umask']);

                /* Aggregate the route file */
                status(STATUS_NOTICE, "Aggregating v4 routes.");
                $aggregated_routes = aggregate::routes($routefile);

                /* print result to a file */
                if (($afile = fopen($aggfile, 'w')) == false)
                {
                    status(STATUS_WARNING, "Can not open agg route file, skipping this record.");
                }
                if(is_array($aggregated_routes))
                {
                    foreach($aggregated_routes as $aggroute)
                    {
                        fwrite($afile, $aggroute);
                    }
                }
                fclose($afile);
                utils::update_file_perms($aggfile, $cfg['cfgfiles']['umask']);
            }
        }

        return $resolve_result['asnlist'];
            
    }

    public static function process_as(&$cvs, &$cfg, $asString, $asNumber, $count, $object, $irr, $email, $o_quiet, $o_4, $o_6, $o_cvs)
    {
        if( $o_quiet == 0 )
		    status(STATUS_INFO, "Processing {$asString} [$object] (Record {$count})");

	    /* Figure out if we have an AUT-NUM or an AS-SET, and resolve it */
        status(STATUS_NOTICE, "Querying IRR Object {$object}");
        
        /* call v4 / v6 processing and aggregation */
        $asnlist4 = [];
        $asnlist6 = [];

        if( $o_4 == 1 && $o_6 == 0 )
        {
            status(STATUS_NOTICE, "Fetching v4 routes.");
            $asnlist4 = irrpt::process($cfg, $irr, $asNumber, 4, $object);
        } elseif( $o_6 == 1 && $o_4 == 0 ) {
            status(STATUS_NOTICE, "Fetching v6 routes.");
            $asnlist6 = irrpt::process($cfg, $irr, $asNumber, 6, $object);
        } else {
            status(STATUS_NOTICE, "Fetching v4 routes.");
            $asnlist4 = irrpt::process($cfg, $irr, $asNumber, 4, $object);

            status(STATUS_NOTICE, "Fetching v6 routes.");
            $asnlist6 = irrpt::process($cfg, $irr, $asNumber, 6, $object);
        }

        /* merge files into common v4 & v6 file */
        if($cfg['fetch']['splitbyorigin'])
        {
            foreach($asnlist4 as $asn)
            {
                if(in_array($asn, $asnlist6))
                {
                    foreach(['', 4, 6] as $v)
                    {
                        ${'routefile'.$v} = $cfg['paths']['db'] . preg_replace('/^AS/', '', $asn) . (($v != '') ? ".{$v}" : "");
                        utils::check_file_perms(${'routefile'.$v});
                        ${'aggfile'.$v} = ${'routefile'.$v} . $cfg['aggregate']['suffix'];
                        utils::check_file_perms(${'aggfile'.$v});
                    }
                    
                    utils::concat_files($routefile4, $routefile6, $routefile);
                    utils::concat_files($aggfile4, $aggfile6, $aggfile);

                    utils::update_file_perms($routefile, $cfg['cfgfiles']['umask']);
                    utils::update_file_perms($aggfile, $cfg['cfgfiles']['umask']);
                    
                    /* Perform CVS tracking */
                    if ($o_cvs == 1 )
                    {
	                    status(STATUS_NOTICE, "Tracking data in CVS.");
  	                    $cvs->init($cfg['paths']['cvsroot']);
	                    utils::check_file_perms($cfg['paths']['cvsroot']);

	                    $rev = irrpt::track($cvs, $routefile);
  	                    $rev_a = irrpt::track($cvs, $aggfile);

	                    /* Send e-mail updates */
	                    status(STATUS_NOTICE, "Send update email.");
  	                    switch ($cfg['fetch']['emailonchange']) {
		                    case "both":
			                    irrpt::update_email($cvs, $cfg, $routefile, $email, $asString, $object, $rev, "Full");
			                    irrpt::update_email($cvs, $cfg, $aggfile, $email, $asString, $object, $rev_a, "Aggregated");
			                    break;
		                    case "full":
			                    irrpt::update_email($cvs, $cfg, $routefile, $email, $asString, $object, $rev, "Full");
			                    break;
		                    case "aggregate":
			                    irrpt::update_email($cvs, $cfg, $aggfile, $email, $asString, $object, $rev_a, "Aggregated");
                                break;
                            default:
                                break;
	                    }
                    }
                }
            }
        } else {
            foreach(['', 4, 6] as $v)
            {
                ${'routefile'.$v} = $cfg['paths']['db'] . $asNumber;
                utils::check_file_perms(${'routefile'.$v});
                ${'aggfile'.$v} = ${'routefile'.$v} . $cfg['aggregate']['suffix'];
                utils::check_file_perms(${'aggfile'.$v});
            }

            utils::concat_files($routefile4, $routefile6, $routefile);
            utils::concat_files($aggfile4, $aggfile6, $aggfile);

            utils::update_file_perms($routefile, $cfg['cfgfiles']['umask']);
            utils::update_file_perms($aggfile, $cfg['cfgfiles']['umask']);

            /* Perform CVS tracking */
            if ($o_cvs == 1 )
            {
                status(STATUS_NOTICE, "Tracking data in CVS.");
                  $cvs->init($cfg['paths']['cvsroot']);
                utils::check_file_perms($cfg['paths']['cvsroot']);

                $rev = irrpt::track($cvs, $routefile);
                  $rev_a = irrpt::track($cvs, $aggfile);

                /* Send e-mail updates */
                status(STATUS_NOTICE, "Send update email.");
                switch ($cfg['fetch']['emailonchange']) {
                    case "both":
                        irrpt::update_email($cvs, $cfg, $routefile, $email, $asString, $object, $rev, "Full");
                        irrpt::update_email($cvs, $cfg, $aggfile, $email, $asString, $object, $rev_a, "Aggregated");
                        break;
                    case "full":
                        irrpt::update_email($cvs, $cfg, $routefile, $email, $asString, $object, $rev, "Full");
                        break;
                    case "aggregate":
                        irrpt::update_email($cvs, $cfg, $aggfile, $email, $asString, $object, $rev_a, "Aggregated");
                        break;
                    default:
                        break;
                }
            }
        }

        /* Log the ASNs behind this object, for future AS-PATH use */
        $asnlist = array_merge($asnlist4, $asnlist6);
        natsort($asnlist);
        $asnlist = array_values(array_unique($asnlist));

	    $asnfile = $cfg['paths']['db'] . $object . $cfg['fetch']['asn_suffix'];
	    status(STATUS_NOTICE, "Writing ASN list to local database.");
	    if (($output = fopen($asnfile, "w")) == false) 
        {
		    status(STATUS_WARNING, "Can not open ASN list output file for {$asString}, skipping this record.");
	    }
	    for($i = 0; $i < sizeof($asnlist); $i++)
        {
		    fwrite($output, sprintf("%s\n", preg_replace("/[aA][sS]/", "", $asnlist[$i])));
        }
	    fclose($output);
        utils::update_file_perms($asnfile, $cfg['cfgfiles']['umask']);
    }

}
?>