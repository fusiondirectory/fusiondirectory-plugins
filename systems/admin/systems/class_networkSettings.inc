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

    if (is_object($this->dialog)) {
      $this->dialog->save_object();
      return $this->dialog->execute();
    }

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

    /* Check if ip must be given */
    if ($this->IPisMust) {
      if (empty($this->ipHostNumber)) {
        $message[] = msgPool::required(_("IP address"));
      } elseif (!tests::is_ip($this->ipHostNumber)) {
        $message[] = msgPool::invalid(_("IP address"), "", "", "192.168.1.10");
      }
    }

    /* Check if mac is empty */
    if ($this->MACisMust) {
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

    $this->dialog = FALSE;
  }

  /* Return plugin informations for acl handling  */
  static function plInfo()
  {
    return array(
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
