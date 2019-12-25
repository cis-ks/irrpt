<?php
/* Including basic necessary Files */
require_once(rtrim(__DIR__, '/') . '/ipv6.inc');
require_once(rtrim(__DIR__, '/') . '/status.inc');

/**
 * Aggregation Class.
 */
class aggregate {
    /**
     * Aggregation of Routes, given by a File
     *
     * @param string $routefile
     * @return array
     */
    public static function routes($routefile) {
        /* normalize and sort addresses and store in array */
        status(STATUS_NOTICE, "Aggregating routes - sorting...");
        if (($addresses = file($routefile)) == false) {
            status(STATUS_WARNING, "Unable to open route file (might be empty), skipping.");
            return false;
        }

        for ($i = 1; $i < count($addresses); $i++) {
            if(ipv6::_ip_version($addresses[$i]) == '6') {
                /* normalize v6 notation */
                $exploded = explode("/", $addresses[$i]);
                $addresses[$i] = ipv6::_v6_expand($exploded[0]) . "/" . $exploded[1];
            }
        }

        /* sort to position prefixes in order, smaller after bigger ones */
        natsort($addresses);
        $addresses = array_values($addresses);

        /* eliminate overlapping prefixes */
        status(STATUS_NOTICE, "Aggregating routes - eliminating overlapping prefixes...");
        $count = count($addresses);
        $current = explode("/", rtrim($addresses[0]));
        for ($i = 1; $i < $count; $i++) {
            $next = explode("/", rtrim($addresses[$i]));
            /* If we picked up two different IP versions we can move on */
            if (ipv6::_ip_version($current[0]) != ipv6::_ip_version($next[0])) {
                $current = $next;
            } /* If both IP's are v4 */
            elseif (ipv6::_ip_version($current[0]) == '4' and ipv6::_ip_version($next[0]) == '4') {
                
                /* Mask both routes and look for overlaps */
                $a = ip2long($current[0]) & (0xFFFFFFFF << (32 - $current[1]));
                $b = ip2long($next[0]) & (0xFFFFFFFF << (32 - $current[1]));
                if ($a == $b) {
                    status(STATUS_NOTICE, "Removing {$next[0]}/{$next[1]}, fits into: {$current[0]}/{$current[1]}");
                    unset($addresses[$i]);
                }
                else {
                    $current = $next;
                }
            } /* If both IP's are v6 */
            elseif (ipv6::_ip_version($current[0]) == '6' and ipv6::_ip_version($next[0]) == '6') {
                /* Mask both routes and look for overlaps */
                $a = ipv6::_v6_to_network($current[0], $current[1] );
                $b = ipv6::_v6_to_network($next[0], $current[1]);
                if ($a == $b) {
                    status(STATUS_NOTICE, "Removing {$next[0]}/{$next[1]}, fits into: {$current[0]}/{$current[1]}");
                    unset($addresses[$i]);
                }
                else {
                    $current = $next;
                }
            }
            else {
                /* whoopsie */
                status(STATUS_ERROR, "Something went terribly wrong while removing overlapping prefixes, aborting.");
                exit(-1);
            }
        }
        $addresses = array_values($addresses);

        /* aggregate neighboring prefixes until nothing changes anymore */
        status(STATUS_NOTICE, "Aggregating routes - aggregating neighboring prefixes...");
        $changes = 1;
        while ($changes == 1) {
            $changes = 0;
            $count = count($addresses);
            $current = explode("/", rtrim($addresses[0]));
            for ($i = 1; $i < $count; $i++) {
                /**
                 * Since we're deleting from the same array we're looping through, we can
                 * end up trying to access prefixes that no longer exist.  Don't do that in order
                 * to avoid warnings
                 */
                if (!isset($addresses[$i]))
                    continue;

                $next = explode("/", rtrim($addresses[$i]));
                /* If we picked up two different IP versions we can move on */
                if (ipv6::_ip_version($current[0]) != ipv6::_ip_version($next[0])) {
                    $current = $next;
                } /* If both IP's are v4 */ elseif (ipv6::_ip_version($current[0]) == '4' and ipv6::_ip_version($next[0]) == '4') {
                    /* If the masks are the same, mask both routes with mask-1 and look if those are identical */
                    if ($current[1] == $next[1] ) {
                        /**
                         * since we're subtracting the current mask from 32, we need to *increment* the current
                         * mask by 1.  Otherwise, we end up turning a /24 into a /25, which will never work for aggregation */
                        $a = ip2long($current[0]) & (0xFFFFFFFF << (32 - $current[1] + 1));
                        $b = ip2long($next[0]) & (0xFFFFFFFF << (32 - $next[1] + 1));
                        if ($a == $b) {
                            /* Remove both smaller prefixes and add the aggregted one */
                            $ip_a = long2ip($a);
                            $new_mask = $next[1] - 1;
                            $new_prefix = $ip_a .  "/" . $new_mask;
                            status(STATUS_WARNING, "Aggregating {$current[0]}/{$current[1]} and {$next[0]}/{$next[1]} into {$new_prefix}");
                            unset($addresses[$i - 1]);
                            $addresses[$i] = "{$new_prefix}\n";
                            $changes = 1;
                        }
                        else {
                            $current = $next;
                        }
                    }
                    else {
                        $current = $next;
                    }
                } /* If both IP's are v6 */ elseif (ipv6::_ip_version($current[0]) == '6' and ipv6::_ip_version($next[0]) == '6') {
                    /* If the masks are the same, mask both routes with mask-1 and look if those are identical */
                    if ($current[1] == $next[1] ) {
                        $a = ipv6::_v6_to_network($current[0], $current[1] - 1);
                        $b = ipv6::_v6_to_network($next[0], $next[1] - 1);
                        if ($a == $b) {
                            /* Remove both smaller prefixes and add the aggregted one */
                            $new_mask = $next[1] - 1;
                            $new_prefix = $a .  "/" . $new_mask;
                            status(STATUS_WARNING, "Aggregating {$current[0]}/{$current[1]} and {$next[0]}/{$next[1]} into {$new_prefix}");
                            unset($addresses[$i - 1]);
                            $addresses[$i] = "{$new_prefix}\n";
                            $changes = 1;
                        }
                        else {
                            $current = $next;
                        }
                    }
                    else {
                        $current = $next;
                    }
                }
                else {
                    /* whoopsie */
                    status(STATUS_ERROR, "Something went terribly wrong while aggregating prefixes, aborting.");
                    exit(-1);
                }
            }
            $addresses = array_values($addresses);
        }

        for ($i = 0; $i < count($addresses); $i++) {
            if (ipv6::_ip_version($addresses[$i]) == '6') {
                /* compress v6 notation */
                $exploded = explode("/", $addresses[$i]);
                $addresses[$i] = ipv6::_v6_compress($exploded[0]) . "/" . $exploded[1];
            }
        }

        return $addresses; 
    }

}