<?php
// Collect inventory and store them in the LDAP
// Usage :
//  fusioninventory-agent --server http://server/fusiondirectory/collect.php
require_once('../include/php_setup.inc');
require_once('functions.inc');
require_once('variables.inc');

$http_raw_post_data = file_get_contents('php://input');

if (!$http_raw_post_data) {
    return;
}

$compressmode = 'none';
if (strpos($http_raw_post_data, "<?xml") === 0) {
    $xml = $http_raw_post_data;
} elseif ($xml = @gzuncompress($http_raw_post_data)) {
    $compressmode = "gzcompress";
} elseif ($xml = @gzinflate ("\x1f\x8b\x08\x00\x00\x00\x00\x00".$http_raw_post_data)) {
    // ** OCS agent 2.0 Compatibility, but return in gzcompress
    $compressmode = "gzdeflate";
    if (strstr($xml, "<QUERY>PROLOG</QUERY>") && !strstr($xml, "<TOKEN>")) {
        $compressmode = "gzcompress";
    }
} elseif ($xml = @gzinflate (substr($http_raw_post_data, 2))) {
    // ** OCS agent 2.0 Compatibility, but return in gzcompress
    $compressmode = "gzdeflate";
    if (strstr($xml, "<QUERY>PROLOG</QUERY>") && !strstr($xml, "<TOKEN>")) {
        $compressmode = "gzcompress";
    }
} else {
    $f = tempnam('/tmp', 'gz_fix');
    file_put_contents($f, "\x1f\x8b\x08\x00\x00\x00\x00\x00".$http_raw_post_data);
    $xml = file_get_contents('compress.zlib://'.$f);

    unlink($f);

    if (strpos($xml, "<?xml") === 0) {
        $compressmode = "gzcompress";
    } else {
        $xml = '';
    }
}

if (strpos($xml, "<?xml") !== 0) {
    error_log("failed to extract XML content");
}

$reply = "";
if (preg_match('/QUERY>PROLOG<\/QUERY/', $xml)) {
    $reply= '<?xml version="1.0" encoding="UTF-8"?>
        <REPLY>
           <RESPONSE>SEND</RESPONSE>
           <PROLOG_FREQ>8</PROLOG_FREQ>
        </REPLY>';
} else {
    $reply = '<?xml version="1.0" encoding="UTF-8"?>
       <REPLY>
       </REPLY>';

    $data = xml::xml2array($xml, 1);
    $data = $data['REQUEST']['CONTENT'];
    $cpus = $data['CPUS'];
    if (!is_numeric(key($cpus))) {
      $cpus = array($cpus);
    }
    $os = $data['OPERATINGSYSTEM'];
    if (!is_numeric(key($os))) {
      $os = array($os);
    }

    /* Check if CONFIG_FILE is accessible */
    if (!is_readable(CONFIG_DIR.'/'.CONFIG_FILE)) {
      die(sprintf(_('FusionDirectory configuration %s/%s is not readable. Aborted.'), CONFIG_DIR, CONFIG_FILE));
    }

    $macs = array();
    $ips  = array();
    foreach ($data['NETWORKS'] as $network) {
      if (isset($network['MACADDR']) && ($network['MACADDR'] != '00:00:00:00:00:00')) {
        $macs[] = $network['MACADDR'];
      }
      if (isset($network['IPADDRESS']) && ($network['IPADDRESS'] != '127.0.0.1')) {
        $ips[]  = $network['IPADDRESS'];
      }
      if (isset($network['IPADDRESS6']) && ($network['IPADDRESS6'] != '::1')) {
        $ips[]  = $network['IPADDRESS6'];
      }
    }
    $macs = array_values(array_unique($macs));
    $ips  = array_values(array_unique($ips));

    /* Parse configuration file */
    $config = new config(CONFIG_DIR.'/'.CONFIG_FILE, $BASE_DIR);
    /* Set config server to default one */
    $directory = $config->data['MAIN']['DEFAULT'];
    if (!isset($config->data['LOCATIONS'][$directory])) {
      $directory = key($config->data['LOCATIONS']);
    }
    $config->set_current($directory);
    session::global_set('config', $config);
    $ldap = $config->get_ldap_link();

    $dn = 'cn='.$_SERVER['REMOTE_ADDR'].','.get_ou('inventoryRDN').$config->current['BASE'];
    $ldap->cat($dn);

    $msg = "";
    if ($ldap->count()) {
      /* Emtpy the subtree */
      $ldap->rmdir_recursive($dn);
      if (!$ldap->success()) {
        $msg.="error :".$ldap->get_error()."\n";
      }
    } else {
      /* Make sure branch is existing */
      $ldap->cd($config->current['BASE']);
      $ldap->create_missing_trees(get_ou('inventoryRDN').$config->current['BASE']);
    }
    /* Create root node */
    $ldap->cd($dn);
    $ldap->add(
      array(
        'cn'                        => $_SERVER['REMOTE_ADDR'],
        'objectClass'               => array('fdInventoryContent'),
        'macAddress'                => $macs,
        'ipHostNumber'              => $ips,
        'fdInventoryVERSIONCLIENT'  => $data['VERSIONCLIENT'],
      )
    );
    if (!$ldap->success()) {
      $msg.="error :".$ldap->get_error()."\n";
    }
    unset($data['VERSIONCLIENT']);
    unset($data['HARDWARE']['ARCHNAME']);

    foreach ($data as $key => $objects) {
      if (!is_numeric(key($objects))) {
        $objects = array($objects);
      }
      foreach ($objects as $i => $object) {
        $cn         = strtolower($key).$i;
        $ldap_attrs = array(
          'cn' => $cn,
          'objectClass' => 'fdInventory'.preg_replace('/_/', '', $key),
        );
        foreach ($object as $attr => $value) {
          if (!(is_array($value) && empty($value))) {
            $ldap_attrs['fdInventory'.preg_replace('/_/', '', $attr)] = $value;
          }
        }
        $ldap->cd('cn='.$cn.','.$dn);
        $ldap->add($ldap_attrs);
        if (!$ldap->success()) {
          $msg .= print_r($ldap_attrs, TRUE);
          $msg .= "error :".$ldap->get_error()."\n";
        }
      }
    }

    if (!empty($msg)) {
      $invFile = sprintf('/tmp/%s.xml', $_SERVER['REMOTE_ADDR']);
      if (!file_put_contents($invFile, "$dn\n$msg\n")) {
          error_log("Failed to write ");
      }
      http_response_code(500);
      print($msg);
      exit();
    }
}

switch($compressmode) {
    case 'none':
        print $reply;
        break;

    case 'gzcompress':
        print gzcompress($reply);
        break;

    case 'gzencode':
        print gzencode($reply);
        break;

    case 'gzdeflate':
        print gzdeflate($reply);
        break;

}
