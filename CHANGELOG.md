Change Log
-------

3.0 - Jan 2020
 * Add: Adding possibility to use an executable config-file as script (using external resources like Netbox)
 * Add: Adding Script as Notification, not E-Mail
 * Add: Splitting the ASses from AS-Set into multiple files to do filtering based on origin AS
 * Enh: Creating several static functions
 * Enh: Chaning Functions for v4/v6 into the same function
 * Enh: Adding several more documentation
 * Fix: Changing design of comments and functions to be the same across all files
 
2.1 - Dec 2019

 * Add: pfxgen added support for iosXR
 * Add: pfxgen added support for Ubiquity EdgeOS (@mikenowak)
 * Add: pfxgen added support for openbgpd
 * Add: pfxgen added support for Huawei (@miuvlad)
 * Enh: more help options
 * Enh: AS Object name is now fetched when run via the CLI by specifying a single AS number
 * Fix: undefined offset in aggreagte.inc
 * Fix: cosmetic issue on completed notices
 * Fix: remove Cisco prefix-list associated with proper address family
 * Fix: prefix lengths not properly checked (juniper, iosxr, f10, extreme)
 * Fix: Force10 pfxgen syntax issue
 * Fix: Email notifications comparing full routue and full agg file (@gawul)
 * Fix: iosXR pfxgen generates blank entry for prefixes not matching length (@bonev)
 * Fix: PHP 7.2.0 behaviour change using count(), fixed errors under Juniper pfxgen
 * Fix: Mask php errors for connection timeout, report proper host timeout.
 * Fix: reduced connection timeout to 15s, down from 30s.
 * Fix: typo in iosxr/ciscoxr (@Bierchermuesli)

2.0 - Aug / Sep / Oct 2015

Changes for version 2.0 by Anna Claiborne <domino@theshell.org>:

 * Tagging new version as 2.0
 * Removed system calls from configure.php
 * Updated configure.php print out to be more easily parsible/readable
 * Updated readme docs for version 2.0
 * Created configure.php for initial setup and cvs directory restore
 * Fixed Force10 prefix list syntax in pfxgen
 * Fixed bugs found in running with PHP 7
 * Removed extra and eroneous space from generated emails
 * Fixed irrpt_fetch to check file ownership before attempting permission
   changes
 * Removed calls to system to concatenate v4/v6 route files.  Now performed by 
   php function in utils.inc
 * Provided support to leave email in irrdb.conf blank if the user wishes no 
   email updates for a particular as/object
 * Fixed support for separate (correct) v4 and v6 prefix list for Juniper config.
 * Added AS validation/checking for pfxgen

Changes for version 2.0 by Elisa Jasinska <elisa@bigwaveit.org>:

 * Added v6 support to IRR query, to prefix exclusion via
   exclusions.conf, to aggregation and to the prefix generator
 * Implemented aggregate functionality and removed dependency on the
   external agregate tool
 * -4 and -6 switches for all command line tools
 * Renamed irrpt_eval and irrpt_explorer into irrpt_list_ases
   and irrpt_list_prefixes
 * Added -f option to provide location to irrdb.conf file
 * Added --nocvs option to omit cvs tracking
 * Bug fix for irrpt_list_ases with -6/-4
 * Improved as number vs as string handling in irrpt_fetch
 * Updated config for correct default cvs path
 * Improved ioverall as/as-set input format checking
 * Changed irrpt_list_prefixes to provide aggregated v6 routes in compressed
   form instead of expanded
 * Added command lines options for seperate v4 and v6 perfix list names
 * Added warning when 0 routes found for AS
 * Added v4/v6 command line switches to irrpt_list_ases
 * Better input validation for AS numbers and AS Sets as well as case sensitivity issues 
   resolved

1.28 - June 8 2015

 * Making the latest version compatible with TORIX changes, such as: -q quiet
   mode, timezone support and memory limit support by Elisa Jasinska 
   <elisa@bigwaveit.org>.

1.27 - Feb 8 2008

 * Redesigned (and simplified) the CVS diff code. The options are now
   "fulldiff" (basically the complete output of a normal CVS diff, that
   you could actually use to patch a file), "plusminus" (stripped down
   to only a list of prefix changes), and "english" (aka "Cogent Mode",
   for NOCs who can't quite figure out what the + and - symbols might
   mean). This was supposed to have been done back in 1.20, but this
   version actually makes it work.

1.26 - June 8 2007

 * Added support for Force10 prefix-list format. The style is JUST
   different enough from Cisco to be incompatible, you have to enter a
   hierarchy under the prefix-list declaration and then enter the permit
   lines seperately, rather than stating it all on one line. Thanks to
   Greg Hankins for pointing this out and providing the fix.

 * Added support for 32-bit (4-byte) ASNs. The #.# format (who's stupid
   idea was that anyways :P) was being detected as an IP address and the
   result of a route-set. Added an additional check to make it detect as
   an object to be be further queried, not that it will help since it
   seems the IRRd software (which is the only protocol we support, for
   various reasons) doesn't currently like importing this format either,
   but at least it won't be IRRPT's fault when it doesn't work. :)

   Thanks to: those wonderful euros who just had to actually go and try
   those new ASNs out ASAP, thus killing all the Farce10 and JUNOS 8.3R1
   boxes on the planet. Go go gadget incompatibility. :)

1.25 - May 27 2006

 * Added an option to automatically change uid/gid to a specified id if
   run as root. CVS will not let you run as root, so you should have a
   dedicated user anyways, but sometimes people forget.

 * Completely changed the way we process arguments, php getopt() sucks.

 * Fixed a plethoa of bugs in irrpt_nag. I don't know how you guys were
   even using this before, or why no one told me how broken this was
   before now. :)

 * Changed the default subject line of irrpt_nag, put the ASN info in
   the front for a quicker read on narrow terminals.


1.24 - December 29 2005

 * Added an "ASN list" file which records the ASNs behind each object. This
   may be used to implement some kind of AS-PATH filtering in the future I
   suppose, though it really isn't the right tool for the job. Talked into
   this evilness by Jon Nistor <nistor@snickers.org>.

 * Fixed a really silly mistake in the processing of "english" style output.

 * Added a new tool "irrpt_explorer", which queries and displays the
   contents of an AS-SET in a hierarchal and recursive format.

 * Removed some unnecessary code in irrpt_eval.

1.23 - November 18 2005

 * Changed $_SERVER['SCRIPT_FILENAME'] references to __FILE__ to work around
   some portability issues with certain PHP builds (like apparently, RedHat).
   Pointed out by Joshua Sahala <jejs@sahala.org>.

 * Fixed the -p flag (preview mode) on irrpt_nag. It works better if you
   actually have "p" in getopt(). Pointed out by Christian Malo
   <chris@fiberpimp.net>.

1.22 - November 8 2005

 * Fixed irrpt_eval -a (aggregate) functionality. Pointed out by
   Jon Nistor <nistor@snickers.org>.

 * Added support for ExtremeWare prefix generation. Submitted by Tom
   Hodgson.

 * A documentation tweak noting that PHP >= 4.3 is required. Submitted
   by Tom Hodgson.

 * Added a note regarding Debian's "aggregate" package to the README.

 * Got a little carried away getting rid of Nistor's strict errors. Rolling
   some things back to continue supporting php 4, easier solution is just
   not to turn on E_STRICT.

 * Added some example scripts for router configuration deployment. Updated
   documentation to reflect these changes.

 * Nailed down a few bugs with caching results that had incorrect array
   indexes following sort and unique. Extensive troubleshooting by Jon
   Nistor <nistor@snickers.org>.

 * Added a quick optimization to only parse the exclusions file once (well
   most of the time at any rate).

 * A couple of minor bugfixes and documentation changes from the previous
   release.


1.20 - November 7 2005

 * Added support for handling a route-set object, and revised the as-set code
   to be a little more generic/graceful while handling it. Issue noted by
   Jon Nistor <nistor@snickers.org>.

 * Added a new tool "irrpt_eval" which returns a simple plain-text list of
   routes from an arbitrary IRR object, using the irrpt query engine. This
   can be useful for diagnostics/quick lookups, and is similar to the
   IRRToolSet tool "peval". Requested by Aaron Weintraub.

 * Fixed some class definitions to comply with php strict mode. Contributed
   by Jon Nistor <nistor@snickers.org>.

 * When an IRR query fails, the query that resulted in the failure is now
   displayed (verbose mode required). Nagged to death by Jon Nistor
   <nistor@snickers.org>.

 * Added a config option for controlling the e-mail updates which are sent
   when a route change is detected:

   irrpt.conf: ['fetch']['emailonchange']

   User can now specify whether to send emails for updated detected in the
   "full" (unaggregated) route file, the "aggregated" route file, "both"
   (default), or "none". Requested by Jon Nistor <nistor@snickers.org>

 * Added an option for "english" language diff format. Apparently the Cogent
   NOC can't figure out what "+" and "-" means, so "english" mode changes
   the output to "Add" and "Remove" (and is the new default). There is
   also an option to continue using + and - ("plusminus") but to remove the
   full "diff" headers. The old behavior is retained under the setting
   "fulldiff". Requested by Adam Rothschild.

   irrpt.conf: ['diff']['output_format']

 * Added a new flag "-p" to irrpt_nag, which enables preview mode. In
   preview mode, the contents of the email(s) which would be sent are
   output to stdout instead of being emailed. This allows you to
   double-check the e-mails before actually sending them. Requested by
   Adam Rothschild.

 * Changed the format of ['pfxgen']['default_pfxname'], and renamed it to
   default_pfxstr. It is now handled as a printf()-style format string, with
   one argument (the ASN). For example, to generate "CUSTOMER:1234" use the
   string: "CUSTOMER:%d". Requested by Pierfrancesco Caci.

 * Updated the distributed (but commented out) example exclusions.conf, just
   incase someone decides to uncomment it without obtaining the current
   exclusions list themselves.

 * A bunch of small misc. changes in formatting and alignment. Largely
   requested by Jon Nistor <nistor@snickers.org>


1.10 - December 29 2004

 * Added an optional local cache for prefixes queried from an aut-num
   record, under the assumption that many networks will have as-set
   records which contain overlapping aut-num records. This will increase
   memory usage a bit, but results in significantly faster queries from
   the IRR whois servers for those with a large number of IRRDB entries.
   Cache is enabled by default. Suggested by Arnold Nipper.

 * Fixed a bug in the default CVS files which would cause an error
   message when fetch is run from a directory other than the default
   "/usr/local/irrpt".

 * Commented out the default bogon routes in the distributed exclusions
   config file, so users must choose to explicitly enable it. Hopefully
   this will prevent the blind use of potentially out of date bogon
   information, and avoid unnecessary whining on mailing lists every
   time a RIR is allocated a new /8.


1.00 - December 26 2004

 * Initial public release.
