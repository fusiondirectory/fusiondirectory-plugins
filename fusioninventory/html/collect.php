<?php
# Collect inventory and store them in $dumpDir
# Usage :
#  fusioninventory-agent --server http://server/collect.php
require_once("../include/php_setup.inc");
require_once("functions.inc");
require_once("variables.inc");

$dumpDir = '/tmp';

##########################################################################
$http_raw_post_data = file_get_contents('php://input');

if (!$http_raw_post_data) {
    return;
}

$compressmode = 'none';
if (strpos($http_raw_post_data, "<?xml") === 0) {
    $xml = $http_raw_post_data;
} else if ($xml = @gzuncompress($http_raw_post_data)) {
    $compressmode = "gzcompress";
} else if ($xml = @gzinflate ("\x1f\x8b\x08\x00\x00\x00\x00\x00".$http_raw_post_data)) {
    // ** OCS agent 2.0 Compatibility, but return in gzcompress
    $compressmode = "gzdeflate";
    if (strstr($xml, "<QUERY>PROLOG</QUERY>")
            AND !strstr($xml, "<TOKEN>")) {
        $compressmode = "gzcompress";
    }
} else if ($xml = @gzinflate (substr($http_raw_post_data, 2))) {
    // ** OCS agent 2.0 Compatibility, but return in gzcompress
    $compressmode = "gzdeflate";
    if (strstr($xml, "<QUERY>PROLOG</QUERY>")
            AND !strstr($xml, "<TOKEN>")) {
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
    $reply = "<?xml version='1.0' encoding='UTF-8'?>
       <REPLY>
       </REPLY>";
    $invFile = sprintf('%s/%s.xml', $dumpDir, $_SERVER['REMOTE_ADDR']);

    $infos = array('fdCpus' => array());

    $data = xml::xml2array($xml, 1);
    $cpus = $data['REQUEST']['CONTENT']['CPUS'];
    if (!is_numeric(key($cpus))) {
      $cpus = array($cpus);
    }
    foreach ($cpus as $cpu) {
      $infos['fdCpus'][] = join('|', array_map(
        function ($key, $value)
        {
          return $key.":".$value;
        },
        array_keys($cpu), array_values($cpu)
      ));
    }
    $os = $data['REQUEST']['CONTENT']['OPERATINGSYSTEM'];
    if (is_numeric(key($os))) {
      $os = $os[0];
    }
    $infos['fdOperatingSystem'] = join('|', array_map(
      function ($key, $value)
      {
        return $key.":".$value;
      },
      array_keys($os), array_values($os)
    ));

    /* Check if CONFIG_FILE is accessible */
    if (!is_readable(CONFIG_DIR."/".CONFIG_FILE)) {
      die(_("FusionDirectory configuration %s/%s is not readable. Aborted."));
    }

    $macs = array();
    foreach ($data['REQUEST']['CONTENT']['NETWORKS'] as $network) {
      if (isset($network['MACADDR']) && ($network['MACADDR'] != "00:00:00:00:00:00")) {
        $macs[] = $network['MACADDR'];
      }
    }
    $macs = array_unique($macs);

    /* Parse configuration file */
    $config = new config(CONFIG_DIR."/".CONFIG_FILE, $BASE_DIR);
    /* Set config server to default one */
    $directory = $config->data['MAIN']['DEFAULT'];
    if (!isset($config->data['LOCATIONS'][$directory])) {
      $directory = key($config->data['LOCATIONS']);
    }
    $config->set_current($directory);
    session::global_set('config', $config);
    $ldap   = $config->get_ldap_link();
    $ldap->search('(|(macAddress='.join(')(macAddress=', $macs).'))');
    if ($attrs = $ldap->fetch()) {
      $dn = $attrs['dn'];

      if (!in_array('fdInventoryObject', $attrs['objectClass'])) {
        $infos['objectClass'] = $attrs['objectClass'];
        unset($infos['objectClass']['count']);
        $infos['objectClass'][] = 'fdInventoryObject';
      }
      $ldap->cd($dn);
      $ldap->modify($infos);
    }

    if (!file_put_contents($invFile, "$dn\n".print_r($data['REQUEST']['CONTENT']['OPERATINGSYSTEM'], TRUE))) {
        error_log("Failed to write ");
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
