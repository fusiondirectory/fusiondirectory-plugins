<?php
// Collect inventory and store them in the LDAP
// Usage :
//  fusioninventory-agent --server http://server/fusiondirectory/collect.php
require_once('../include/php_setup.inc');
require_once('functions.inc');
require_once('variables.inc');

$http_raw_post_data = file_get_contents('php://input');

function returnError ($errorText)
{
  $protocol = (isset($_SERVER['SERVER_PROTOCOL']) ? $_SERVER['SERVER_PROTOCOL'] : 'HTTP/1.0');
  header($protocol.' 500 '.$errorText);
  error_log('Error while collecting inventory: '.$errorText);
  exit();
}

if (!$http_raw_post_data) {
  return;
}

$compressmode = 'none';
if (strpos($http_raw_post_data, "<?xml") === 0) {
  $xml = $http_raw_post_data;
} elseif ($xml = @gzuncompress($http_raw_post_data)) {
  $compressmode = "gzcompress";
} elseif ($xml = @gzinflate("\x1f\x8b\x08\x00\x00\x00\x00\x00".$http_raw_post_data)) {
  // ** OCS agent 2.0 Compatibility, but return in gzcompress
  $compressmode = "gzdeflate";
  if (strstr($xml, "<QUERY>PROLOG</QUERY>") && !strstr($xml, "<TOKEN>")) {
    $compressmode = "gzcompress";
  }
} elseif ($xml = @gzinflate(substr($http_raw_post_data, 2))) {
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
  $reply = '<?xml version="1.0" encoding="UTF-8"?>
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

  /* Check if CONFIG_FILE is accessible */
  if (!is_readable(CONFIG_DIR.'/'.CONFIG_FILE)) {
    returnError(sprintf(_('FusionDirectory configuration %s/%s is not readable. Aborted.'), CONFIG_DIR, CONFIG_FILE));
  }

  $macs = [];
  $ips  = [];
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

  if ($ldap->count()) {
    /* Emtpy the subtree */
    $ldap->rmdir_recursive($dn);
    if (!$ldap->success()) {
      returnError('LDAP: '.$ldap->get_error());
    }
  } else {
    /* Make sure branch is existing */
    $ldap->cd($config->current['BASE']);
    try {
      $ldap->create_missing_trees(get_ou('inventoryRDN').$config->current['BASE']);
    } catch (FusionDirectoryError $error) {
      returnError($error->getMessage());
    }
  }
  /* Create root node */
  $ldap->cd($dn);
  $ldap->add(
    [
      'cn'                        => $_SERVER['REMOTE_ADDR'],
      'objectClass'               => ['fdInventoryContent'],
      'macAddress'                => $macs,
      'ipHostNumber'              => $ips,
      'fdInventoryVERSIONCLIENT'  => $data['VERSIONCLIENT'],
    ]
  );
  if (!$ldap->success()) {
    returnError('LDAP: '.$ldap->get_error());
  }
  unset($data['VERSIONCLIENT']);
  unset($data['HARDWARE']['ARCHNAME']);

  foreach ($data as $key => $objects) {
    if (!is_numeric(key($objects))) {
      $objects = [$objects];
    }
    foreach ($objects as $i => $object) {
      $cn         = strtolower($key).$i;
      $ldap_attrs = [
        'cn' => $cn,
        'objectClass' => 'fdInventory'.preg_replace('/_/', '', $key),
      ];
      foreach ($object as $attr => $value) {
        if (is_array($value)) {
          if (empty($value)) {
            continue;
          }
          if (!is_numeric(key($value))) {
            // Invalid value, json_encode it to get string value
            $value = json_encode($value);
          } else {
            // Make sure indexes are continuous
            $value = array_values($value);
            // Make sure values are strings
            foreach ($value as $k => $v) {
              if (!is_string($v)) {
                // Invalid value, json_encode it to get string value
                $value[$k] = json_encode($v);
              }
            }
          }
        }
        $ldap_attrs['fdInventory'.preg_replace('/_/', '', $attr)] = $value;
      }
      $ldap->cd('cn='.$cn.','.$dn);
      $r = $ldap->add($ldap_attrs);
      if (!$ldap->success()) {
        // We cannot stop at every error, or one missing objectClass in the schema will stop inventory
        error_log('LDAP: '.$ldap->get_error());
      } elseif (!$r) {
        error_log('LDAP: Failed to insert '.$key);
      }
    }
  }
}

switch ($compressmode) {
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
