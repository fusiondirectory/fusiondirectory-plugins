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
    $data = $data['REQUEST']['CONTENT'];
    $cpus = $data['CPUS'];
    if (!is_numeric(key($cpus))) {
      $cpus = array($cpus);
    }
    $os = $data['OPERATINGSYSTEM'];
    if (!is_numeric(key($os))) {
      $os = array($os);
    }

    $macs = array();
    foreach ($data['NETWORKS'] as $network) {
      if (isset($network['MACADDR']) && ($network['MACADDR'] != "00:00:00:00:00:00")) {
        $macs[] = $network['MACADDR'];
      }
    }
    $macs = array_unique($macs);

    /* Check if CONFIG_FILE is accessible */
    if (!is_readable(CONFIG_DIR."/".CONFIG_FILE)) {
      die(sprintf(_("FusionDirectory configuration %s/%s is not readable. Aborted."), CONFIG_DIR, CONFIG_FILE));
    }

    /* Parse configuration file */
    $config = new config(CONFIG_DIR."/".CONFIG_FILE, $BASE_DIR);
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
      /* TODO */
    } else {
      /* Create root node */
      $ldap->cd($config->current['BASE']);
      $ldap->create_missing_trees(get_ou('inventoryRDN').$config->current['BASE']);
      $ldap->cd($dn);
      $ldap->add(
        array(
          'cn'          => $_SERVER['REMOTE_ADDR'],
          'objectClass' => array('fdInventoryContent')
        )
      );
    }
    foreach ($cpus as $i => $cpu) {
      $ldap->cd('cn=cpu'.$i.','.$dn);
      $ldap->add(
        array(
          'cn' => 'cpu'.$i,
          'objectClass'             => array('fdInventoryCpu'),
          'fdInventoryName'         => $cpu['NAME'],
          'fdInventoryCore'         => $cpu['CORE'],
          'fdInventoryFamilyNumber' => $cpu['FAMILYNUMBER'],
          'fdInventoryManufacturer' => $cpu['MANUFACTURER'],
          'fdInventoryModel'        => $cpu['MODEL'],
          'fdInventorySpeed'        => $cpu['SPEED'],
          'fdInventoryStepping'     => $cpu['STEPPING'],
          'fdInventoryThread'       => $cpu['THREAD'],
        )
      );
    }

    if (!file_put_contents($invFile, "$dn\n".print_r($data, TRUE))) {
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
