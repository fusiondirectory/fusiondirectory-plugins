<?php
/**
 * sieve-php.lib.php
 *
 * $Id: sieve-php.lib.php,v 1.5 2003/12/18 13:23:19 avel Exp $
 *
 * Copyright 2001-2003 Dan Ellis <danellis@rushmore.com>
 *
 * This program is released under the GNU Public License.  See the enclosed
 * file COPYING for license information. If you did not receive this file, see
 * http://www.fsf.org/copyleft/gpl.html.
 *
 * You should have received a copy of the GNU Public License along with this
 * package; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * See CHANGES for updates since last release
 *
 * @author Dan Ellis
 * @package sieve-php
 * @copyright Copyright 2002-2003, Dan Ellis, All Rights Reserved.
 * @version 0.1.0
 */

/**
 * Constants
 */
define("F_NO", 0);
define("F_OK", 1);
define("F_DATA", 2);
define("F_HEAD", 3);

define("EC_NOT_LOGGED_IN", 0);
define("EC_QUOTA", 10);
define("EC_NOSCRIPTS", 20);
define("EC_UNKNOWN", 255);

/**
 * SIEVE class - A Class that implements MANAGESIEVE in PHP4|5.
 *
 * This program provides a handy interface into the Cyrus timsieved server
 * under php4.  It is tested with Sieve server included in Cyrus 2.0, but it
 * has been upgraded (not tested) to work with older Sieve server versions.
 *
 * NOTE: a major change since version (0.0.5) is the inclusion of a standard
 * method to retrieve  server responses.  All functions will return either TRUE
 * or FALSE and will fill in $sieve->error with a defined error code like
 * EC_QUOTA, raw server errors in $sieve->error_raw, and successful responses
 * in $sieve->responses.
 *
 * Usage is pretty simple.  The basics is login, do what you need and logout.
 * There are two sample files (which suck) test.php and testsieve.php.
 * test.php allows you to create/delete/view scripts and testsieve.php is a
 * very basic sieve server test.
 *
 * Please let us know of any bugs, problems or ideas at sieve-php development
 * list:  sieve-php-devel@lists.sourceforge.net. A web interface to subscribe
 * to this list is available at:
 * https://lists.sourceforge.net/mailman/listinfo/sieve-php-devel
 *
 * @author Dan Ellis
 * @example simple_example.php A simple example that shows usage of sieve-php
 * class.
 * @example vacationset-sieve.php A more elaborate example of vacation script
 * handling.
 * @version 0.1.0
 * @package sieve-php
 * @todo Maybe add the NOOP function.
 * @todo Have timing mechanism when port problems arise.
 * @todo Provide better error diagnostics.
 */


class sieve
{
  var $host;
  var $port;
  var $user;
  var $pass;
  /**
   * a comma seperated list of allowed auth types, in order of preference
   */
  var $auth_types;
  /**
   * type of authentication attempted
   */
  var $auth_in_use;

  var $line;
  var $fp;
  var $retval;
  var $tmpfile;
  var $fh;
  var $len;
  var $script;

  var $loggedin;
  var $capabilities;
  var $error;
  var $error_raw;
  var $responses;

  var $token;
  var $mtoken;
  var $rtoken;
  var $ltoken;
  var $item;
  var $x;
  var $err_len;
  var $err_recv;
  var $response;
  var $auth;
  var $found_script;
  var $cap_type;
  var $modules;
  var $module;

  protected $tls;

  //maybe we should add an errorlvl that the user will pass to new sieve = sieve(,,,,E_WARN)
  //so we can decide how to handle certain errors?!?

  /**
   * get response
   */
  function get_response ()
  {
    if ($this->loggedin == FALSE or feof($this->fp)) {
      $this->error      = EC_NOT_LOGGED_IN;
      $this->error_raw  = "You are not logged in.";
      return FALSE;
    }

    unset($this->response);
    unset($this->error);
    unset($this->error_raw);

    $this->line   = fgets($this->fp, 1024);
    $this->token  = explode(" ", $this->line, 2);

    if ($this->token[0] == "NO") {
      /* we need to try and extract the error code from here.  There are two possibilites: one, that it will take the form of:
         NO ("yyyyy") "zzzzzzz" or, two, NO {yyyyy} "zzzzzzzzzzz" */
      $this->x = 0;
      list($this->ltoken, $this->mtoken, $this->rtoken) = explode(" ", $this->line." ", 3);
      if ($this->mtoken[0] == "{") {
        while ($this->mtoken[$this->x] != "}" or $this->err_len < 1) {
          $this->err_len = substr($this->mtoken, 1, $this->x);
          $this->x++;
        }
        $this->line = fgets($this->fp, $this->err_len);
        $this->error_raw[] = substr($this->line, 0, strlen($this->line) - 2);    //we want to be nice and strip crlf's
        $this->err_recv = strlen($this->line);

        /* Ensure max loop of 1000, to keep the ui from freezing */
        $max = 1000;
        $cur = 0;

        while ($this->err_recv < ($this->err_len) && ($cur < $max)) {
          $cur ++;
          $this->line         = fgets($this->fp, 4096);
          $this->err_recv     += strlen($this->line);
          $this->error_raw[]  = preg_replace("/\r\n/", "", $this->line);    //we want to be nice and strip crlf'
        }
      } elseif ($this->mtoken[0] == "(") {
        switch ($this->mtoken) {
          case "(\"QUOTA\")":
            $this->error      = EC_QUOTA;
            $this->error_raw  = $this->rtoken;
            break;
          default:
            $this->error      = EC_UNKNOWN;
            $this->error_raw  = $this->rtoken;
            break;
        } /* end switch */
      } else {
        $this->error      = EC_UNKNOWN;
        $this->error_raw  = $this->line;
      }
      return FALSE;
    } elseif (substr($this->token[0], 0, 2) == "OK") {
      return TRUE;
    } elseif ($this->token[0][0] == "{") {

      /* Unable wild assumption:  that the only function that gets here is the get_script(), doesn't really matter though */

      /* the first line is the len field {xx}, which we don't care about at this point */
      $this->line = fgets($this->fp, 1024);
      while (substr($this->line, 0, 2) != "OK" and substr($this->line, 0, 2) != "NO") {
        $this->response[] = $this->line;
        $this->line = fgets($this->fp, 1024);
      }
      if (substr($this->line, 0, 2) == "OK") {
        return TRUE;
      } else {
        return FALSE;
      }
    } elseif ($this->token[0][0] == "\"") {

      /* I'm going under the _assumption_ that the only function that will get here is the listscripts().
         I could very well be mistaken here, if I am, this part needs some rework */

      $this->found_script = FALSE;

      while (substr($this->line, 0, 2) != "OK" and substr($this->line, 0, 2) != "NO") {
        $this->found_script = TRUE;
        list($this->ltoken, $this->rtoken) = explode(" ", $this->line, 2);

        if (strcmp(rtrim($this->rtoken), "ACTIVE") == 0) {
          $this->response["ACTIVE"] = substr(rtrim($this->ltoken), 1, -1);
        } else {
          $this->response[] = substr(rtrim($this->ltoken), 1, -1);
        }
        $this->line = fgets($this->fp, 1024);
      } /* end while */

      return TRUE;
    } else {
      $this->error      = EC_UNKNOWN;
      $this->error_raw  = $this->line;
      print '<b><i>UNKNOWN ERROR (Please report this line to <a
      href="mailto:sieve-php-devel@lists.sourceforge.net">sieve-php-devel
      Mailing List</a> to include in future releases):
      '.$this->line.'</i></b><br>';

      return FALSE;
    } /* end else */
  } /* end get_response() */

  /**
   * Initialization of the SIEVE class.
   *
   * This also loads some arrays up with some handy information:
   *
   * @param string $host hostname to connect to. Usually the IMAP server where
   * a SIEVE daemon, such as timsieved, is listening.
   *
   * @param string $port Numeric port to connect to. SIEVE daemons usually
   * listen to port 2000.
   *
   * @param string $user is a super-user or proxy-user that has ACL rights to
   * login on behalf of the $auth.
   *
   * @param string $pass password to use for authentication
   *
   * @param string $auth is the authorized user identity for which the SIEVE
   * scripts will be managed.
   *
   * @param string $auth_types a string containing all the allowed
   * authentication types allowed in order of preference, seperated by spaces.
   * (ex.  "PLAIN DIGEST-MD5 CRAM-MD5"  The method the library will try first
   * is PLAIN.) The default for this value is PLAIN.
   *
   * Note: $user, if included, is the account name (and $pass will be the
   * password) of an administrator account that can act on behalf of the user.
   * If you are using Cyrus, you must make sure that the admin account has
   * rights to admin the user.  This is to allow admins to edit/view users
   * scripts without having to know the user's password.  Very handy.
   */
  function __construct ($host, $port, $user, $pass, $auth = "", $options = "", $auth_types = "PLAIN DIGEST-MD5")
  {
    $this->host = $host;
    $this->port = $port;
    $this->user = $user;
    $this->pass = $pass;
    if (!strcmp($auth, "")) {   /* If there is no auth user, we deem the user itself to be the auth'd user */
      $this->auth = $this->user;
    } else {
      $this->auth = $auth;
    }
    $this->auth_types   = $auth_types;  /* Allowed authentication types */
    $this->fp           = 0;
    $this->line         = "";
    $this->retval       = "";
    $this->tmpfile      = "";
    $this->fh           = 0;
    $this->len          = 0;
    $this->capabilities = [];
    $this->loggedin     = FALSE;
    $this->error        = "";
    $this->error_raw    = "";
    $this->tls          = ($options === 'tls');

    @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $this->error_raw,
        "<b>SIEVE: Host: $host:$port. Options: $options - Auth Types: $auth_types </b>");
  }

  /**
   * Tokenize a line of input by quote marks and return them as an array
   *
   * @param string $string Input line to parse for quotes
   * @return array|false Array of broken by quotes parts of original string
   */
  function parse_for_quotes ($string)
  {
    $start = -1;
    $index = 0;

    for ($ptr = 0; $ptr < strlen($string); $ptr++) {
      if ($string[$ptr] == '"' and $string[$ptr] != '\\') {
        if ($start == -1) {
          $start = $ptr;
        } else {
          $token[$index++] = substr($string, $start + 1, $ptr - $start - 1);
          $found = TRUE;
          $start = -1;
        } /* end else */
      } /* end if */
    } /* end for */

    if (isset($token)) {
      return $token;
    } else {
      return FALSE;
    }
  } /* end function */

  /**
   * Parser for status responses.
   *
   * This should probably be replaced by a smarter parser.
   *
   * @param string $string Input that contains status responses.
   * @todo remove this function and dependencies
   */
  function status ($string)
  {
    /*  Need to remove this and all dependencies from the class */

    switch (substr($string, 0, 2)) {
      case "NO":
        return F_NO;    //there should be some function to extract the error code from this line
        //NO ("quota") "You are only allowed x number of scripts"
      case "OK":
        return F_OK;
      default:
        switch ($string[0]) {
          case "{":
            //do parse here for curly braces - maybe modify
            //parse_for_quotes to handle any parse delimiter?
            return F_HEAD;
          default:
            return F_DATA;
        }
    }
  }

  /**
   * Attempt to log in to the sieve server.
   *
   * It will return FALSE if it fails, TRUE if all is well.  This also loads
   * some arrays up with some handy information:
   *
   * capabilities["implementation"] contains the sieve version information
   *
   * capabilities["auth"] contains the supported authentication modes by the
   * SIEVE server.
   *
   * capabilities["modules"] contains the built in modules like "reject",
   * "redirect", etc.
   *
   * capabilities["starttls"] , if is set and equal to TRUE, will show that the
   * server supports the STARTTLS extension.
   *
   * capabilities["unknown"] contains miscellaneous/extraneous header info sieve
   * may have sent
   *
   * @return boolean
   */
  function sieve_login ()
  {
    $this->fp = @fsockopen($this->host, $this->port, $err_id, $err_str);
    if ($this->fp == FALSE) {
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $this->error_raw,
          "<b>SIEVE: Socket connection failed. (".$this->host.":".$this->port.")</b>");

      $this->error_raw = $err_str;
      return FALSE;
    }

    $this->line = fgets($this->fp, 1024);

    //Hack for older versions of Sieve Server.  They do not respond with the Cyrus v2. standard
    //response.  They respond as follows: "Cyrus timsieved v1.0.0" "SASL={PLAIN,........}"
    //So, if we see IMPLEMENTATION in the first line, then we are done.

    if (strstr($this->line, "IMPLEMENTATION")) {
      //we're on the Cyrus V2 sieve server
      while (sieve::status($this->line) == F_DATA) {
        $this->item = sieve::parse_for_quotes($this->line);

        if (strcmp($this->item[0], "IMPLEMENTATION") == 0) {
          $this->capabilities["implementation"] = $this->item[1];
        } elseif (strcmp($this->item[0], "SIEVE") == 0 or strcmp($this->item[0], "SASL") == 0) {
          if (strcmp($this->item[0], "SIEVE") == 0) {
            $this->cap_type = "modules";
          } else {
            $this->cap_type = "auth";
          }

          $this->modules = explode(" ", $this->item[1]);
          foreach ($this->modules as $this->module) {
            $this->capabilities[$this->cap_type][$this->module] = TRUE;
          }
        } elseif (strcmp($this->item[0], "STARTTLS") == 0) {
          $this->capabilities['starttls'] = TRUE;
        } else {
          $this->capabilities["unknown"][] = $this->line;
        }
        $this->line = fgets($this->fp, 1024);
      }// end while
    } else {
      //we're on the older Cyrus V1. server
      //this version does not support module reporting.  We only have auth types.
      $this->cap_type = "auth";

      //break apart at the "Cyrus timsieve...." "SASL={......}"
      $this->item = sieve::parse_for_quotes($this->line);

      $this->capabilities["implementation"] = $this->item[0];

      //we should have "SASL={..........}" now.  Break out the {xx,yyy,zzzz}
      $this->modules = substr($this->item[1], strpos($this->item[1], "{"), strlen($this->item[1]) - 1);

      //then split again at the ", " stuff.
      $this->modules = explode(", ", $this->modules);

      //fill up our $this->modules property
      if (is_array($this->modules)) {
        foreach ($this->modules as $this->module) {
          $this->capabilities[$this->cap_type][$this->module] = TRUE;
        }
      } elseif (is_string($this->modules)) {
        $this->capabilities[$this->cap_type][$this->module] = TRUE;
      }
    }

    if (sieve::status($this->line) == F_NO) {   //here we should do some returning of error codes?
      $this->error = EC_UNKNOWN;

      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $this->error_raw,
          "<b>SIEVE: Unknown sieve response, giving up</b>");

      $this->error_raw = "Server not allowing connections.";
      return FALSE;
    }

    /* decision login to decide what type of authentication to use... */

    /* Fill error message if no auth types are present */
    if (!$this->tls && !isset($this->capabilities["auth"])) {
      $this->error      = EC_UNKNOWN;
      $this->error_raw  = "No authentication methods found - please check your sieve setup for missing sasl modules";
      return FALSE;
    }

    /* Loop through each allowed authentication type and see if the server allows the type */
    foreach (explode(" ", $this->auth_types) as $auth_type) {
      if (isset($this->capabilities["auth"][$auth_type])) {
        /* We found an auth type that is allowed. */
        $this->auth_in_use = $auth_type;
        break;
      }
    }

    /* Sometimes we do not get any authentification methods, assume PLAIN here.
     * #TODO Maybe there is a better solution for this?
     */
    if ($this->auth_in_use == "") {
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $this->error_raw,
          "<b>SIEVE: No authentification mechanisms received, try using PLAIN!</b>");
      $this->auth_in_use = "PLAIN";
    }

    /* DEBUG */
    $imp = "";
    if (isset($this->capabilities['implementation'])) {
      $imp = $this->capabilities['implementation'];
    }
    @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $imp,
        "<b>SIEVE: Socket connection established</b>");

    /* call our authentication program */
    return sieve::authenticate();
  }

  /**
   * Log out of the sieve server.
   *
   * @return boolean Always returns TRUE at this point.
   */
  function sieve_logout ()
  {
    if ($this->loggedin == FALSE) {
      return FALSE;
    }

    fputs($this->fp, "LOGOUT\r\n");
    fclose($this->fp);

    @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "Logout!", "<b>SIEVE</b>");

    $this->loggedin = FALSE;
    return TRUE;
  }

  /**
   * Send the script contained in $script to the server.
   *
   * It will return any error results it finds (in $sieve->error and
   * $sieve->error_raw), and return TRUE if it is successfully sent.  The
   * function does _not_ automatically make the script the active script.
   *
   * @param string $scriptname The name of the SIEVE script.
   * @param string $script The script to be uploaded.
   * @return boolean Returns TRUE if script has been successfully uploaded.
   */
  function sieve_sendscript ($scriptname, $script)
  {
    if ($this->loggedin == FALSE) {
      return FALSE;
    }
    $this->script = stripslashes($script);
    $len = strlen($this->script);
    fputs($this->fp, "PUTSCRIPT \"$scriptname\" {".$len."+}\r\n");
    fputs($this->fp, "$this->script\r\n");

    if (sieve::get_response()) {
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $this->line,
          "<b>SIEVE: Script '".$scriptname."' successfully sent</b>");
      return TRUE;
    } else {
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, trim($this->error_raw),
          "<b>SIEVE: Script couldn't be sent</b>");
      return FALSE;
    }
  }

  /**
   * Check if there is enough space for a script to be uploaded.
   *
   * This function returns TRUE or FALSE based on whether the sieve server will
   * allow your script to be sent and your quota has not been exceeded.  This
   * function does not currently work due to a believed bug in timsieved.  It
   * could be my code too.
   *
   * It appears the timsieved does not honor the NUMBER type.  see lex.c in
   * timsieved src.  don't expect this function to work yet.  I might have
   * messed something up here, too.
   *
   * @param string $scriptname The name of the SIEVE script.
   * @param integer $scriptsize The size of the SIEVE script.
   * @return boolean
   * @todo Does not work; bug fix and test.
   */
  function sieve_havespace ($scriptname, $scriptsize)
  {
    if ($this->loggedin == FALSE) {
      return FALSE;
    }
    fputs($this->fp, "HAVESPACE \"$scriptname\" $scriptsize\r\n");
    return sieve::get_response();
  }

  /**
   * Set the script active on the sieve server.
   *
   * @param string $scriptname The name of the SIEVE script.
   * @return boolean
   */
  function sieve_setactivescript ($scriptname)
  {
    if ($this->loggedin == FALSE) {
      return FALSE;
    }

    fputs($this->fp, "SETACTIVE \"$scriptname\"\r\n");

    if (sieve::get_response()) {
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $this->line,
          "<b>SIEVE: Set active script '".$scriptname."' was successful</b>");
      return TRUE;
    } else {
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, trim($this->error_raw),
          "<b>SIEVE: Set active script '".$scriptname."' failed</b>");
      return FALSE;
    }
  }

  /**
   * Return the contents of the requested script.
   *
   * If you want to display the script, you will need to change all CrLf to
   * '.'.
   *
   * @param string $scriptname The name of the SIEVE script.
   * @return bool SIEVE script data.
   */
  function sieve_getscript ($scriptname)
  {
    unset($this->script);
    if ($this->loggedin == FALSE) {
      return FALSE;
    }

    fputs($this->fp, "GETSCRIPT \"$scriptname\"\r\n");

    if (sieve::get_response()) {
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $this->line,
          "<b>SIEVE: Get script '".$scriptname."' was successful</b>");
      return TRUE;
    } else {
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, trim($this->error_raw),
          "<b>SIEVE: Get script '".$scriptname."' failed</b>");
      return FALSE;
    }
  }

  /**
   * Attempt to delete the script requested.
   *
   * If the script is currently active, the server will not have any active
   * script after the deletion.
   *
   * @param string $scriptname The name of the SIEVE script.
   * @return mixed
   */
  function sieve_deletescript ($scriptname)
  {
    if ($this->loggedin == FALSE) {
      return FALSE;
    }

    fputs($this->fp, "DELETESCRIPT \"$scriptname\"\r\n");

    if (sieve::get_response()) {
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $this->line,
          "<b>SIEVE: Delete script '".$scriptname."' was successful</b>");
      return TRUE;
    } else {
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, trim($this->error_raw),
          "<b>SIEVE: Delete  script '".$scriptname."' failed</b>");
      return FALSE;
    }
  }


  /**
   * List available scripts on the SIEVE server.
   *
   * This function returns TRUE or FALSE.  $sieve->response will be filled
   * with the names of the scripts found.  If a script is active, the
   * $sieve->response["ACTIVE"] will contain the name of the active script.
   *
   * @return boolean
   */
  function sieve_listscripts ()
  {
    fputs($this->fp, "LISTSCRIPTS\r\n");
    sieve::get_response();    //should always return TRUE, even if there are no scripts...
    if (isset($this->found_script) and $this->found_script) {
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, implode(", ", $this->response),
          "<b>SIEVE: List scripts was successful</b>");
      return TRUE;
    } else {
      $this->error      = EC_NOSCRIPTS;  //sieve::getresponse has no way of telling wether a script was found...
      $this->error_raw  = "No scripts found for this account.";
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $this->error_raw,
          "<b>SIEVE: List scripts returned no scripts</b>");
      return FALSE;
    }
  }


  /**
   * Check availability of connection to the SIEVE server.
   *
   * This function returns TRUE or FALSE based on whether the connection to the
   * sieve server is still alive.
   *
   * @return boolean
   */
  function sieve_alive ()
  {
    if (!isset($this->fp) or $this->fp == 0) {
      $this->error = EC_NOT_LOGGED_IN;
      return FALSE;
    } elseif (feof($this->fp)) {
      $this->error = EC_NOT_LOGGED_IN;
      return FALSE;
    } else {
      return TRUE;
    }
  }

  /**
   * Perform SASL authentication to SIEVE server.
   *
   * Attempts to authenticate to SIEVE, using some SASL authentication method
   * such as PLAIN or DIGEST-MD5.
   *
   */
  function authenticate ()
  {
    /* Check if a TLS bases connection is required */
    if ($this->tls) {
      /* Initiate TLS and get response */
      if (@fputs($this->fp, "STARTTLS\r\n") === FALSE) {
        @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "couldn't send data to socket.",
            "<b>SIEVE: TLS couldn't be initiated</b>");
        $this->error_raw = "fputs() returned 'FALSE'";
        return FALSE;
      }
      $this->line = @fgets($this->fp, 1024);
      if ($this->line === FALSE) {
        $this->error_raw = "fgets() returned 'FALSE'";
        @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "couldn't read data from socket.",
            "<b>SIEVE: TLS couldn't be initiated</b>");
        return FALSE;
      }
      if (sieve::status($this->line) != F_OK) {
        @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "server returned '".trim($this->line)."' instead of 'OK'",
            "<b>SIEVE: TLS couldn't be initiated</b>");
        $this->error_raw = "expected an 'OK', but server replied: '" . trim($this->line) . "'";
        return FALSE;
      }
      if (!@stream_socket_enable_crypto($this->fp, TRUE, STREAM_CRYPTO_METHOD_TLS_CLIENT)) {
        @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__,
            "The socket doesn't seem to support STREAM_CRYPTO_METHOD_TLS_CLIENT",
            "<b>SIEVE: TLS couldn't be initiated</b>");
        $this->error_raw = "stream_socket_enable_crypto() returned 'FALSE'";
        return FALSE;
      }
      @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $this->line, "<b>SIEVE: TLS successfully initiated</b>");
    }

    switch ($this->auth_in_use) {

      case "PLAIN":
        $auth = base64_encode("$this->auth\0$this->user\0$this->pass");

        $this->len = strlen($auth);
        fputs($this->fp, "AUTHENTICATE \"PLAIN\" {".$this->len."+}\r\n");
        fputs($this->fp, "$auth\r\n");

        $this->line = fgets($this->fp, 1024);
        while (sieve::status($this->line) == F_DATA) {
          $this->line = fgets($this->fp, 1024);
        }

        if (sieve::status($this->line) == F_NO) {
          $this->error_raw = $this->line;
          @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, trim($this->error_raw),
              '<b>SIEVE: Authentication for "'.$this->user.'" ('.$this->auth_in_use.') failed</b>');

          return FALSE;
        }

        @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $this->line,
            '<b>SIEVE: Authentication for "'.$this->user.'" ('.$this->auth_in_use.') was successful</b>');
        $this->loggedin = TRUE;
        return TRUE;

      case "DIGEST-MD5":
        // SASL DIGEST-MD5 support works with timsieved 1.1.0
        // follows rfc2831 for generating the $response to $challenge
        fputs($this->fp, "AUTHENTICATE \"DIGEST-MD5\"\r\n");
        // $clen is length of server challenge, we ignore it.
        $clen = fgets($this->fp, 1024);
        // read for 2048, rfc2831 max length allowed
        $challenge = fgets($this->fp, 2048);
        // vars used when building $response_value and $response
        $cnonce = base64_encode(bin2hex(static::hmac_md5(microtime())));
        $ncount = "00000001";
        $qop_value = "auth";
        $digest_uri_value = "sieve/".$this->host;
        // decode the challenge string
        $result = static::decode_challenge($challenge);
        // verify server supports qop=auth
        $qop = explode(",", $result['qop']);
        if (!in_array($qop_value, $qop)) {
          // rfc2831: client MUST fail if no qop methods supported
          $this->error_raw = 'rfc2831: client MUST fail if no qop methods supported';
          @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, trim($this->error_raw),
              '<b>SIEVE: Authentication for "'.$this->user.'" ('.$this->auth_in_use.') failed</b>');
          return FALSE;
        }
        // build the $response_value
        $string_a1 = utf8_encode($this->user).":";
        $string_a1 .= utf8_encode($result['realm']).":";
        $string_a1 .= utf8_encode($this->pass);
        $string_a1 = static::hmac_md5($string_a1);
        $A1 = $string_a1.":".$result['nonce'].":".$cnonce.":".utf8_encode($this->auth);
        $A1 = bin2hex(static::hmac_md5($A1));
        $A2 = bin2hex(static::hmac_md5("AUTHENTICATE:$digest_uri_value"));
        $string_response = $result['nonce'].":".$ncount.":".$cnonce.":".$qop_value;
        $response_value = bin2hex(static::hmac_md5($A1.":".$string_response.":".$A2));
        // build the challenge $response
        $reply = "charset=utf-8,username=\"".$this->user."\",realm=\"".$result['realm']."\",";
        $reply .= "nonce=\"".$result['nonce']."\",nc=$ncount,cnonce=\"$cnonce\",";
        $reply .= "digest-uri=\"$digest_uri_value\",response=$response_value,";
        $reply .= "qop=$qop_value,authzid=\"".utf8_encode($this->auth)."\"";
        $response = base64_encode($reply);
        fputs($this->fp, "\"$response\"\r\n");

        $this->line = fgets($this->fp, 1024);
        while (sieve::status($this->line) == F_DATA) {
          $this->line = fgets($this->fp, 1024);
        }

        if (sieve::status($this->line) == F_NO) {
          $this->error_raw = $this->line;
          @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, trim($this->error_raw),
              '<b>SIEVE: Authentication for "'.$this->user.'" ('.$this->auth_in_use.') failed</b>');
          return FALSE;
        }
        $this->loggedin = TRUE;
        @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $this->line,
            '<b>SIEVE: Authentication for "'.$this->user.'" ('.$this->auth_in_use.') was successful</b>');
        return TRUE;

      case "CRAM-MD5":
        // SASL CRAM-MD5 support works with timsieved 1.1.0
        // follows rfc2195 for generating the $response to $challenge
        // CRAM-MD5 does not support proxy of $auth by $user
        if ($this->auth != $this->user) {
          $this->error_raw = 'CRAM-MD5 does not support proxy of $auth by $user';
          @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, trim($this->error_raw),
              '<b>SIEVE: Authentication for "'.$this->user.'" ('.$this->auth_in_use.') aborted</b>');
          return FALSE;
        }
        // requires php mhash extension
        fputs($this->fp, "AUTHENTICATE \"CRAM-MD5\"\r\n");
        // $clen is the length of the challenge line the server gives us
        $clen = fgets($this->fp, 1024);
        // read for 1024, should be long enough?
        $challenge = fgets($this->fp, 1024);
        // build a response to the challenge
        $hash = bin2hex(static::hmac_md5(base64_decode($challenge), $this->pass));
        $response = base64_encode($this->user." ".$hash);
        // respond to the challenge string
        fputs($this->fp, "\"$response\"\r\n");

        $this->line = fgets($this->fp, 1024);
        while (sieve::status($this->line) == F_DATA) {
          $this->line = fgets($this->fp, 1024);
        }

        if (sieve::status($this->line) == F_NO) {
          $this->error_raw = $this->line;
          @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, trim($this->error_raw),
              '<b>SIEVE: Authentication for "'.$this->user.'" ('.$this->auth_in_use.') failed</b>');
          return FALSE;
        }
        $this->loggedin = TRUE;
        @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $this->line,
            '<b>SIEVE: Authentication for "'.$this->user.'" ('.$this->auth_in_use.') was successful</b>');
        return TRUE;

      case "LOGIN":
        if ($this->auth != $this->user) {
          $this->error_raw = 'LOGIN does not support proxy of $auth by $user';
          @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, trim($this->error_raw),
              '<b>SIEVE: Authentication for "'.$this->user.'" ('.$this->auth_in_use.') aborted</b>');
          return FALSE;
        }
        $login  = base64_encode($this->user);
        $pass   = base64_encode($this->pass);

        fputs($this->fp, "AUTHENTICATE \"LOGIN\"\r\n");
        fputs($this->fp, "{".strlen($login)."+}\r\n");
        fputs($this->fp, "$login\r\n");
        fputs($this->fp, "{".strlen($pass)."+}\r\n");
        fputs($this->fp, "$pass\r\n");

        $this->line = fgets($this->fp, 1024);
        while (sieve::status($this->line) == F_HEAD ||
              sieve::status($this->line) == F_DATA) {
          $this->line = fgets($this->fp, 1024);
        }

        if (sieve::status($this->line) == F_NO) {
          $this->error_raw = $this->line;
          return FALSE;
        }
        $this->loggedin = TRUE;
        @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, $this->line,
            '<b>SIEVE: Authentication for "'.$this->user.'" ('.$this->auth_in_use.') was successful</b>');
        return TRUE;

      default:
        @DEBUG(DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "Unsupported authentication method '".$this->auth_in_use."'!",
            "<b>SIEVE: Authentication for '".$this->user."' with type '".$this->auth_in_use."' failed</b>");

        $this->error_raw = "Unsupported authentication method '".$this->auth_in_use."'!";
        return FALSE;

    }//end switch
  }

  /**
   * Creates a HMAC digest that can be used for auth purposes.
   * See RFCs 2104, 2617, 2831
   * Uses mhash() extension if available
   *
   * Squirrelmail has this function in functions/auth.php, and it might have been
   * included already. However, it helps remove the dependancy on mhash.so PHP
   * extension, for some sites. If mhash.so _is_ available, it is used for its
   * speed.
   *
   * This function is Copyright (c) 1999-2003 The SquirrelMail Project Team
   * Licensed under the GNU GPL. For full terms see the file COPYING.
   *
   * @param string $data Data to apply hash function to.
   * @param string $key Optional key, which, if supplied, will be used to
   * calculate data's HMAC.
   * @return string HMAC Digest string
   */
  static function hmac_md5 ($data, $key = '')
  {
    // See RFCs 2104, 2617, 2831
    // Uses mhash() extension if available
    if (extension_loaded('mhash')) {
      if ($key == '') {
        $mhash = mhash(MHASH_MD5, $data);
      } else {
        $mhash = mhash(MHASH_MD5, $data, $key);
      }
      return $mhash;
    }
    if (!$key) {
      return pack('H*', md5($data));
    }
    $key = str_pad($key, 64, chr(0x00));
    if (strlen($key) > 64) {
      $key = pack("H*", md5($key));
    }
    $k_ipad = $key ^ str_repeat(chr(0x36), 64);
    $k_opad = $key ^ str_repeat(chr(0x5c), 64);
    /* Heh, let's get recursive. */
    $hmac = static::hmac_md5($k_opad . pack("H*", md5($k_ipad . $data)));
    return $hmac;
  }

  /**
   * A hack to decode the challenge from timsieved 1.1.0.
   *
   * This function may not work with other versions and most certainly won't work
   * with other DIGEST-MD5 implentations
   *
   * @param string $input Challenge supplied by timsieved.
   */
  static function decode_challenge ($input)
  {
    $input = base64_decode($input);
    preg_match("/nonce=\"(.*)\"/U", $input, $matches);
    $resp['nonce'] = $matches[1];
    preg_match("/realm=\"(.*)\"/U", $input, $matches);
    $resp['realm'] = $matches[1];
    preg_match("/qop=\"(.*)\"/U", $input, $matches);
    $resp['qop'] = $matches[1];
    return $resp;
  }
}

