<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2015  FusionDirectory

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class networkSettings extends plugin
{
  /* attribute list for save action */
  var $ignore_account = TRUE;

  /* Basic informations
   */
  var $attributes     = array("ipHostNumber","macAddress");
  var $objectclasses  = array("whatever");

  var $ipHostNumber          = "";    // IP address
  var $additionalHostNumbers = array();
  var $macAddress            = "";    // Mac address

  var $orig_ipHostNumber   = "";    // IP address
  var $orig_macAddress     = "";    // Mac address

  var $OrigCn         = "";    // Initial cn
  var $IPisMust       = TRUE;
  var $MACisMust      = TRUE;
  var $dialog         = FALSE;

  /* DCHP Attributes
   */
  var $dhcpAttributes           = array("dhcpParentNode");
  var $dhcpEnabled              = FALSE;
  var $dhcp_is_Account          = FALSE;
  var $dhcpParentNodes          = array();
  var $dhcpParentNode           = "";
  var $dhcpHostEntry            = array();
  var $initial_dhcpParentNode   = "";
  var $initial_dhcpHostEntry    = array();
  var $initial_dhcp_is_Account  = FALSE;

  var $used_ip_mac              = array();

  var $namingAttr               = "cn";

  function __construct ($parent, $objectClasses, $IPisMust = FALSE, $namingAttr = "cn")
  {
    global $config;
    /* We need to know which objectClasses are used, to store the ip/mac
     * Because of different type of devices
     */
    $this->parent         = $parent;
    $this->objectclasses  = $objectClasses;
    $this->IPisMust       = $IPisMust;
    $this->namingAttr     = $namingAttr;

    parent::__construct($parent->dn, $this->parent);

    $this->attrs = &$this->parent->attrs;

    if (isset($this->attrs[$namingAttr][0])) {
      $this->OrigCn = preg_replace('/\$$/', '', $this->attrs[$namingAttr][0]);
    }

    /* Create list of additional ipHostNumber.
     */
    $this->additionalHostNumbers = array();
    if (isset($this->attrs['ipHostNumber']) && $this->attrs['ipHostNumber']['count'] > 1) {
      for ($i = 1; $i < $this->attrs['ipHostNumber']['count']; $i++) {
        $this->additionalHostNumbers[] = $this->attrs['ipHostNumber'][$i];
      }
    }

    /************
     * DHCP
     ************/

    /* Hide all dhcp specific code, if dhcp plugin is not present in config */
    $this->dhcpEnabled = class_available('dhcpHost');

    if ($this->dhcpEnabled) {
      $this->dhcpParentNodes = $this->get_dhcp_parent_nodes();
      $this->dhcpParentNode  = $this->get_dhcp_parent_node();
      if ($this->dhcpParentNode) {
        $this->dhcp_is_Account          = TRUE;
        $this->initial_dhcp_is_Account  = TRUE;
        $this->dhcpHostEntry            = $this->get_dhcp_host_entry();
      }
      $this->initial_dhcpHostEntry  = $this->dhcpHostEntry;
      $this->initial_dhcpParentNode = $this->dhcpParentNode;
    }

    /* Create a list of used mac and ip addresses.

       ! We use this optically huge amount of code to fetch all
       Mac and IP addresses, because a simple search for mac and IP
       over the whole ldap server was 10 to 20 times slower.
     */
    $deps  = array();
    $ou = preg_replace("/,.*$/", "", get_ou("systemRDN"));

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search("(&(objectClass=organizationalUnit)(".$ou."))", array("dn"));
    while ($attrs = $ldap->fetch()) {
        $deps[] = $attrs['dn'];
    }

    foreach ($deps as $dep) {
      $ldap->cd($dep);
      $ldap->search("(|(macAddress=*)(ipHostNumber=*))", array("macAddress","ipHostNumber"));
      while ($attrs = $ldap->fetch()) {
        if (isset($attrs['ipHostNumber'][0])) {
          $this->used_ip_mac["ip:".$attrs['ipHostNumber'][0]] = "ip:".$attrs['ipHostNumber'][0];
        }
        if (isset($attrs['macAddress'][0])) {
          $this->used_ip_mac["mac:".$attrs['macAddress'][0]] = "mac:".$attrs['macAddress'][0];
        }
      }
    }

    /* Save initial ip and mac values, to be able
        check if the used values are already in use */
    $this->orig_ipHostNumber   = $this->ipHostNumber;
    $this->orig_macAddress     = $this->macAddress;
  }

  function resetCopyInfos()
  {
    $this->initial_dhcp_is_Account  = FALSE;
    $this->initial_dhcpHostEntry    = array();
    $this->initial_dhcpParentNode   = "";
    $this->orig_ipHostNumber        = "";
    $this->orig_macAddress          = "";
    $this->OrigCn                   = "";
    parent::resetCopyInfos();
  }

  function getCn()
  {
    $namingAttr = $this->namingAttr;
    return preg_replace('/\$$/', '', $this->parent->$namingAttr);
  }

  function getVarsForSaving($attrs)
  {
    foreach ($this->attributes as $attr) {
      if (!empty($this->$attr)) {
        $attrs[$attr] = $this->$attr;
      }
    }
    return $attrs;
  }

  function execute()
  {
    /* Call parent execute */
    $smarty = get_smarty();
    $smarty->assign("MACisMust", $this->MACisMust);

    $tmp = $this->plInfo();
    foreach ($tmp['plProvidedAcls'] as $name => $translation) {
      $smarty->assign($name."ACL", $this->getacl($name));
    }

    $display = "";

    /**********
     * Additional ipHostNumber handling
     **********/

    /* Add a new one */
    if ($this->acl_is_writeable("ipHostNumber")) {
      foreach ($_POST as $name => $value) {
        if (preg_match("/^additionalHostNumbers_add/", $name)) {
          $this->additionalHostNumbers[] = "";
          break;
        }

        /* Delete given entry */
        if (preg_match("/^additionalHostNumbers_del_/", $name)) {
          $id = preg_replace("/^^additionalHostNumbers_del_([0-9]*)_.*/", "\\1", $name);
          if (isset($this->additionalHostNumbers[$id])) {
            unset($this->additionalHostNumbers[$id]);
            $this->additionalHostNumbers = array_values($this->additionalHostNumbers);
          }
          break;
        }
      }
    }

    $smarty->assign("additionalHostNumbers", $this->additionalHostNumbers);

    /**********
     * DHCP Handling
     **********/

    if (isset($_POST['dhcpEditOptions']) && $this->acl_is_readable("dhcpSetup")) {
      if (count($this->dhcpHostEntry) == 0) {
        $this->dialog = new dhcpHost($this->parent, $this->dhcpParentNode, TRUE);
      } else {
        $this->dialog = new dhcpHost($this->parent, $this->dhcpHostEntry, TRUE);
      }
      $this->dialog->cn   = $this->getCn();
      $this->dialog->read_only     = !$this->acl_is_writeable("dhcpSetup");
      $this->dialog->dhcpHWAddress = "ethernet ".$this->macAddress;
      if (!empty($this->ipHostNumber)) {
        $this->dialog->statements->set('fixed-address', $this->ipHostNumber);
      }
    }

    if (isset($_POST['cancel_dhcp'])) {
      $this->dialog = FALSE;
    }

    if (isset($_POST['save_dhcp']) && $this->acl_is_writeable("dhcpSetup") && is_object($this->dialog)) {
      $this->dialog->save_object();
      $msgs = $this->dialog->check(array());
      if (count($msgs)) {
        foreach ($msgs as $msg) {
          msg_dialog::display(_("Error"), $msg, ERROR_DIALOG);
        }
      } else {
        $this->dhcpHostEntry = $this->dialog->save();
        $this->dialog = FALSE;
      }
    }

    if (is_object($this->dialog)) {
      $this->dialog->save_object();
      return $this->dialog->execute();
    }

    $smarty->assign("dhcpEnabled",        $this->dhcpEnabled && $this->acl_is_readable("dhcpSetup"));
    $smarty->assign("dhcp_is_Account",    $this->dhcp_is_Account);
    $smarty->assign("dhcpParentNode",     $this->dhcpParentNode);
    $smarty->assign("dhcpParentNodes",    $this->dhcpParentNodes);
    $smarty->assign("dhcpParentNodeCnt",  count($this->dhcpParentNodes));

    /* Is IP address must ? */
    $smarty->assign("IPisMust", $this->IPisMust);

    /* Assign smarty all attributes */
    foreach ($this->attributes as $attr) {
      $smarty->assign($attr, $this->$attr);
    }

    $display .= $smarty->fetch(get_template_path('network.tpl', TRUE));

    return $display;
  }


  function remove_from_parent()
  {
    global $config;
    $ldap = $config->get_ldap_link();

    /************
     * DHCP Handling
     ************/

    /* DHCP removed */
    if ($this->dhcp_is_Account) {
      $ldap->rmdir_recursive($this->dhcpHostEntry['dn']);
      if (!$ldap->success()) {
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dhcpHostEntry['dn'], LDAP_DEL, get_class()), LDAP_ERROR);
      }
    }
  }


  /* Save data to object */
  function save_object()
  {
    if (isset($_POST['network_tpl_posted'])) {

      /* Save all posted vars */
      parent::save_object();

      /******
        Additional IP Host Numbers
       ******/

      /* Get posts for all additionally added ipHostNumbers */
      if ($this->acl_is_writeable("ipHostNumber")) {
        foreach ($this->additionalHostNumbers as $id => $value) {
          if (isset($_POST['additionalHostNumbers_'.$id])) {
            $this->additionalHostNumbers[$id] = get_post('additionalHostNumbers_'.$id);
          }
        }
      }

      /******
        DHCP posts
       ******/

      if ($this->acl_is_writeable("dhcpSetup") && $this->dhcpEnabled && isset($_POST['network_tpl_posted'])) {
        foreach ($this->dhcpAttributes as $attr) {
          if (isset($_POST[$attr])) {
            $this->$attr = $_POST[$attr];
          }
        }
        if (isset($_POST['dhcp_is_Account'])) {
          $this->dhcp_is_Account = TRUE;
        } else {
          $this->dhcp_is_Account = FALSE;
        }
      }

      /* Get all attributes (IP/MAC)*/
      foreach ($this->attributes as $attr) {
        if (isset($_POST[$attr]) && $this->acl_is_writeable($attr)) {
          $this->$attr = $_POST[$attr];
        }
      }
    }
  }


  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message = plugin::check();

    /******
      check additional IP Host Numbers
     ******/

    foreach ($this->additionalHostNumbers as $id => $value) {
      if (!tests::is_ip($value)) {
        $message[] = msgPool::invalid(sprintf(_("IP address %s"), ($id + 2)), "", "", "192.168.1.10");
      }
    }

    /* Check if mac and ip are already used */
    if (!empty($this->ipHostNumber) &&
        $this->ipHostNumber != $this->orig_ipHostNumber &&
        in_array("ip:".$this->ipHostNumber, $this->used_ip_mac)) {
      $message[] = msgPool::duplicated(_("IP address"));
    }
    if ($this->dhcp_is_Account) {
      if (!empty($this->macAddress) &&
          $this->macAddress != $this->orig_macAddress &&
          in_array("mac:".$this->macAddress, $this->used_ip_mac)) {
        $message[] = msgPool::duplicated(_("MAC address"));
      }
    }

    /* Check if ip must be given */
    if ($this->IPisMust) {
      if (empty($this->ipHostNumber)) {
        $message[] = msgPool::required(_("IP address"));
      } elseif (!tests::is_ip($this->ipHostNumber)) {
        $message[] = msgPool::invalid(_("IP address"), "", "", "192.168.1.10");
      }
    }

    /* Check if mac is empty */
    if ($this->MACisMust || $this->dhcp_is_Account) {
      if ($this->macAddress == "" ) {
        $message[] = msgPool::required(_("MAC address"));
      } elseif (!tests::is_mac($this->macAddress)) {
        $message[] = msgPool::invalid(_("MAC address"), "", "", "00:0C:7F:31:33:F1");
      }
    }

    return $message;
  }


  /* Save to LDAP */
  function save()
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $dn = $this->parent->dn;
    $cn = $this->getCn();

    /* IP-MAC HANDLING */

    /* $dn was posted as parameter */
    $this->dn = $dn;

    /* Save ip/Mac*/
    parent::save();

    /* Add all additional ipHostNumbers now
     */
    if (!empty($this->ipHostNumber)) {
      $this->attrs['ipHostNumber'] = array($this->ipHostNumber);
    }
    foreach ($this->additionalHostNumbers as $value) {
      $this->attrs['ipHostNumber'][] = $value;
    }

    /* Do not add the objectClass ipHost if no ip address is given */
    if (!isset($this->attrs['ipHostNumber']) || empty($this->attrs['ipHostNumber'])) {
      $this->attrs['objectClass'] = array_remove_entries(array('ipHost'), $this->attrs['objectClass']);
    }

    /* Write back to ldap */
    $ldap->cd($this->dn);
    $this->cleanup();
    $ldap->modify ($this->attrs);

    /* Display errors
     */
    if (!$ldap->success()) {
      msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()), LDAP_ERROR);
    }

    /* DHCP HANDLING */

    /* New entry */
    if ($this->dhcpEnabled && $this->acl_is_writeable('dhcpSetup')) {
      if (count($this->dhcpHostEntry) == 0) {
        $this->dialog = new dhcpHost($this->parent, $this->dhcpParentNode, TRUE);
        $this->dialog->cn = $cn;
        $this->dialog->dhcpHWAddress = 'ethernet '.$this->macAddress;
        if (!empty($this->ipHostNumber)) {
          $this->dialog->statements->set('fixed-address', $this->ipHostNumber);
        }
        $this->dialog->execute();
        $this->dialog->save_object();
        $this->dhcpHostEntry = $this->dialog->save();
        if (count($this->dhcpHostEntry['dhcpOption']) == 0) {
          $this->dhcpHostEntry['dhcpOption'] = array('host-name '.$cn);
        }
      } else {
        // Updated Host-Name entry
        foreach ($this->dhcpHostEntry['dhcpOption'] as $key => $entry) {
          if (preg_match('/^host-name/', $entry)) {
            $this->dhcpHostEntry['dhcpOption'][$key] = 'host-name '.$cn;
            break;
          }
        }
      }

      /* Write mac address to dhcp settings */
      if ($this->dhcp_is_Account) {
        if (!isset($this->dhcpHostEntry['dhcpHWAddress'][0]) ||
            !preg_match('/ethernet '.$this->macAddress.'/', $this->dhcpHostEntry['dhcpHWAddress'][0])) {
          $this->dhcpHostEntry['dhcpHWAddress'] = array('ethernet '.$this->macAddress);
          $this->dhcpHostEntry['MODIFIED'] = TRUE;
        }
      }

      /* Updated IP host number */
      if ($this->dhcp_is_Account && $this->dhcpHostEntry['dhcpStatements']) {
        foreach ($this->dhcpHostEntry['dhcpStatements'] as $id => $value) {
          if (preg_match('/^fixed-address/', $value)) {
            $this->dhcpHostEntry['dhcpStatements'][$id] = 'fixed-address '.$this->ipHostNumber;
            $this->dhcpHostEntry['MODIFIED']            = TRUE;
          }
        }
      }

      /* Unset dhcpStatements if this attribute is empty  */
      if (isset($this->dhcpHostEntry['dhcpStatements']) &&
          ($this->dhcpHostEntry['dhcpStatements'] == "" || count($this->dhcpHostEntry['dhcpStatements']) == 0) ) {
        unset($this->dhcpHostEntry['dhcpStatements']);
      }

      /* DHCP removed */
      if ($this->initial_dhcp_is_Account && !$this->dhcp_is_Account) {
        $ldap->rmdir_recursive($this->dhcpHostEntry['dn']);
        if (!$ldap->success()) {
          msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $this->dhcpHostEntry['dn'], LDAP_DEL, get_class()), LDAP_ERROR);
        }

        $tmp = new serviceDHCP($this->dhcpParentNode);
        $tmp->handle_post_events('remove');
      }

      /* DHCP Added */
      if (!$this->initial_dhcp_is_Account && $this->dhcp_is_Account) {
        $attrs = $this->dhcpHostEntry;
        unset($attrs['MODIFIED']);
        unset($attrs['dn']);
        $ldap->cd('cn='.$cn.','.$this->dhcpParentNode);
        $res = $ldap->add($attrs);

        $tmp = new serviceDHCP($this->dhcpParentNode);
        $tmp->handle_post_events('add');

        if (!$ldap->success()) {
          msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), 'cn='.$cn.','.$this->dhcpParentNode, LDAP_ADD, get_class()), LDAP_ERROR);
        }
      }

      /* DHCP still activated */
      if ($this->initial_dhcp_is_Account && $this->dhcp_is_Account) {

        /* DHCP node changed */
        if (($this->initial_dhcpParentNode != $this->dhcpParentNode) ||
           ($cn != $this->OrigCn)) {
          $attrs        = $this->dhcpHostEntry;
          $attrs['cn']  = $cn;
          unset($attrs['dn']);
          unset($attrs['MODIFIED']);
          $ldap->cd('cn='.$cn.','.$this->dhcpParentNode);
          $res = $ldap->add($attrs);
          if (!$ldap->success()) {
            msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), 'cn='.$cn.','.$this->dhcpParentNode, LDAP_ADD, get_class()), LDAP_ERROR);
          }

          $tmp = new serviceDHCP($this->dhcpParentNode);
          $tmp->handle_post_events('modify');

          if ($res) {
            $ldap->rmdir_recursive($this->dhcpHostEntry['dn']);
            if (!$ldap->success()) {
              msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $this->dhcpHostEntry['dn'], LDAP_DEL, get_class()), LDAP_ERROR);
            }
          }
        }

        /* SAME node but modified */
        if (isset($this->dhcpHostEntry['MODIFIED']) && ($this->dhcpHostEntry['MODIFIED'] == 1) &&
            $this->initial_dhcpParentNode == $this->dhcpParentNode) {
          $attrs = $this->dhcpHostEntry;
          unset($attrs['dn']);
          unset($attrs['MODIFIED']);
          $ldap->cd($this->dhcpHostEntry['dn']);
          $ldap->modify($attrs);

          $tmp = new serviceDHCP($this->dhcpParentNode);
          $tmp->handle_post_events('modify');

          if (!$ldap->success()) {
            msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $this->dhcpHostEntry['dn'], LDAP_MOD, get_class()), LDAP_ERROR);
          }
        }
      }
    }
    $this->dialog = FALSE;
  }

  /* Return plugin informations for acl handling  */
  static function plInfo()
  {
    $tmp = array(
      'plShortName'   => _('Network'),
      'plDescription' => _('Network settings'),
      'plSelfModify'  => FALSE,
      'plPriority'    => 5,
      'plCategory'    => array('workstation', 'terminal', 'phone', 'server', 'component', 'printer', 'winstation'),

      'plProvidedAcls' => array(
        'ipHostNumber'  => _('IP address'),
        'macAddress'    => _('MAC address')
      )
    );

    /* Hide all dhcp configuration if not available */
    if (class_available('serviceDHCP')) {
      $tmp['plProvidedAcls']['dhcpSetup'] = _('DHCP configuration');
    }
    return $tmp;
  }


  function get_dhcp_host_entry()
  {
    global $config;
    $attrs = array();
    $dn = $this->get_dhcp_host_entry_dn();
    if ($dn) {
      $ldap = $config->get_ldap_link();
      $ldap->cd($config->current['BASE']);
      $ldap->cat($dn, array('*'));
      if ($ldap->count()) {
        $attrs = $ldap->fetch();
        foreach ($attrs as $key => $value) {
          if (is_numeric($key) || ($key == 'count')) {
            unset($attrs[$key]);
          }
          if (is_array($value) && isset($value['count'])) {
            unset($attrs[$key]['count']);
          }
        }
      }
    }
    return $attrs;
  }


  function get_dhcp_host_entry_dn()
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search ('(&(objectClass=dhcpHost)(cn='.$this->getCn().'))', array('uid','cn','dn'));

    if ($ldap->count()) {
      $attr = $ldap->fetch();
      return $attr['dn'];
    } else {
      return '';
    }
  }


  function get_dhcp_parent_node()
  {
    return preg_replace('/^cn='.preg_quote($this->getCn(), '/').',/', '', $this->get_dhcp_host_entry_dn());
  }

  function get_dhcp_parent_nodes()
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=dhcpService)', array('dhcpPrimaryDN'));

    $dhcp_dns = array();
    while ($attr = $ldap->fetch()) {
      $dhcp_dns[$attr['dn']] = $attr['dhcpPrimaryDN'][0];
    }

    foreach ($dhcp_dns as $key => $pri_dns) {
      $ldap->cat($pri_dns, array('cn'));
      $tmp = $ldap->fetch();
      if (isset($tmp['cn'][0])) {
        $dhcp_dns[$key] = $tmp['cn'][0];
      } else {
        unset($dhcp_dns[$key]);
      }
    }

    $tmp = $tmp2 = array();
    foreach ($dhcp_dns as $dn => $cn) {
      $ldap->cd($dn);
      $ldap->search('(|(objectClass=dhcpService)(objectClass=dhcpGroup)'.
                    '(objectClass=dhcpSubnet)(objectClass=dhcpSharedNetwork))', array('cn'));
      while ($attr = $ldap->fetch()) {
        $tmp[$attr['dn']] = $attr['cn'][0];
      }
      $tmp2 = array_merge($tmp2, $this->create_tree($tmp, preg_replace('/^[^,]+,/i', '', $dn), '('.$cn.')&nbsp;'));
    }
    return $tmp2;
  }

  function generateRandomIP($net = "")
  {
    global $config;
    // first gather all IPs
    $ldap = $config->get_ldap_link();
    $ocs =
      '(objectClass=fdPhone)'.
      '(objectClass=fdMobilePhone)'.
      '(objectClass=goServer)'.
      '(objectClass=GOhard)'.
      '(objectClass=gotoTerminal)'.
      '(objectClass=gotoWorkstation)'.
      '(objectClass=gotoPrinter)'.
      '(objectClass=ipHost)';
    $list = array();
    $ldap->search("(&(|{$ocs})(ipHostNumber=*))", array("ipHostNumber"));
    while ($attrs = $ldap->fetch()) {
      if (preg_match("/^$net\./", $attrs['ipHostNumber'][0])) {
        $list[] = $attrs['ipHostNumber'][0];
      }
    }

    // Set starting ip.
    $ip_data = preg_split('/\./', $net);
    for ($i = 0;$i < 4;$i++) {
      if (!isset($ip_data[$i])) {
        $ip_data[$i] = 0;
      }
    }

    // Search the next free and valid ip.
    while (in_array(implode('.', $ip_data), $list) || $ip_data[3] <= 1) {
      $ip_data[3]++;
      if ($ip_data[3] >= 255) {
        $ip_data[3] = 1;
        $ip_data[2]++;
      }
      if ($ip_data[2] >= 255) {
        $ip_data[2] = 1;
        $ip_data[1]++;
      }
      if ($ip_data[1] >= 255) {
        $ip_data[1] = 1;
        $ip_data[0]++;
      }
      if ($ip_data[0] >= 255) {
        break;
      }
    }

    return implode('.', $ip_data);
  }


  function create_tree($arr, $base, $current = '')
  {
    $ret = array();
    foreach ($arr as $r => $name) {
      $base_part = str_replace($base, '', $r);
      if (preg_match('/^[a-z]*='.preg_quote($name, '/').'(|,)$/i', $base_part)) {
        $ret[$r] = $current.$name;
        $tmp = $this->create_tree($arr, $r, $current.'.&nbsp;');
        foreach ($tmp as $sub_key => $sub_name) {
          $ret[$sub_key] = $sub_name;
        }
      }
    }
    return $ret;
  }
}

class NetworkSettingsDialog
{
  private $attribute;
  private $display;

  function __construct (&$attribute, &$display)
  {
    $this->attribute  = &$attribute;
    $this->display    = &$display;
  }

  function execute ()
  {
    $this->display = $this->attribute->netConfigDNS->execute();
    if (is_object($this->attribute->netConfigDNS->dialog)) {
      return $this->display;
    }
    return FALSE;
  }
}

class NetworkSettingsAttribute extends Attribute
{
  public $netConfigDNS;
  protected $display = NULL;
  protected $namingAttr;
  protected $ocs;

  function __construct ($namingAttr = 'cn', $ocs = array())
  {
    parent::__construct('Network Settings', '', 'networkSettings', FALSE, '');
    $this->setInLdap(FALSE);
    $this->namingAttr = $namingAttr;
    $this->ocs        = $ocs;
  }

  function setManagedAttributes ($dontcare)
  {
    trigger_error('method setManagedAttributes is not supported for NetworkSettingsAttribute');
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      $this->netConfigDNS = new networkSettings($this->plugin,
        array_merge($this->plugin->objectclasses, $this->ocs),
        TRUE, $this->namingAttr
      );
      $this->plugin->netConfigDNS = $this->netConfigDNS;
    }
  }

  function renderAttribute(&$attributes, $readOnly)
  {
    if ($this->display === NULL) {
      $this->display = $this->netConfigDNS->execute();
    }
    $attributes[$this->getLdapName()] = array(
      'display'     => $this->display
    );
  }

  function loadPostValue ()
  {
  }

  function applyPostValue ()
  {
    if (!$this->disabled) {
      /* Refresh base */
      $this->netConfigDNS->save_object();
      $this->display = $this->netConfigDNS->execute();
      if (is_object($this->netConfigDNS->dialog)) {
        $this->plugin->openDialog(new NetworkSettingsDialog($this, $this->display));
      }
    }
  }

  function check ()
  {
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    } else {
      return $this->netConfigDNS->check();
    }
  }
}

?>
