<?php
/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * Copyright (C) 2010 Antoine Gallavardin
 * Copyright (C) 2011-2013 FusionDirectory
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/*TODO
 * [DONE] Add a UnitIntAttribute for the block soft and hard limit
 * [DONE] Make everything work with fake device list
 * Change the server select so that it submits the form, code the glue to fill the device list properly
 * */

class UnitIntAttribute extends CompositeAttribute
{
  function __construct ($label, $description, $ldapName, $required, $units, $min = FALSE, $max = FALSE, $acl = "", $label = "Composite attribute")
  {
    parent::__construct($description, $ldapName,
      array(
        new IntAttribute('', '', $ldapName.'_int', $required, $min, $max),
        new SelectAttribute('', '', $ldapName.'_select', TRUE, array_keys($units), '', array_values($units)),
      ), '', '',
      $acl, $label
    );
    $this->setLinearRendering(TRUE);
  }

  function readValues($value)
  {
    $mod = 1024;
    for ($i = 0; $value >= $mod; $i++) {
      $value /= $mod;
    }
    return array($value, $i);
  }

  function writeValues($values)
  {
    return $values[0] * pow(1024, $values[1]);
  }

  function displayValue($value)
  {
    $values = $this->readValues($value);
    return $values[0].$this->attributes[1]->displayValue($values[1]);
  }
}

class QuotaEntryAttribute extends CharSeparatedCompositeAttribute
{
  function readValues($value)
  {
    $values = parent::readValues($value);
    $blocksize = $this->plugin->getBlockSize($values[5], $values[0]);
    $values[1] *= $blocksize;
    $values[2] *= $blocksize;
    return $values;
  }

  function writeValues($values)
  {
    if (!empty($values[5])) {
      $blocksize = $this->plugin->getBlockSize($values[5], $values[0]);
      $values[1] /= $blocksize;
      $values[2] /= $blocksize;
    }
    return parent::writeValues($values);
  }
}

class quotaAccount2 extends simplePlugin
{
  var $objectclasses = array("systemQuotas");

  var $displayHeader = TRUE;

  protected $attrQuotaServer = NULL;
  protected $attrQuotaDevice = NULL;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Quota'),
      'plDescription' => _('Plugin for quota support'),
      'plSelfModify'  => TRUE,
      'plPriority'    => 11,
      'plCategory'    => array('users'),
      'plObjectType'  => array('user'),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    $unit_options = array(_('B'),_('KB'),_('MB'),_('GB'),_('TB'));
    return array(
      'main' => array(
        'template'  => get_template_path('quota_section.tpl', TRUE, dirname(__FILE__)),
        'class'     => array('fullwidth'),
        'name'      => _('Quota informations'),
        'icon'      => 'plugins/quota/images/storage.png',
        'attrs'     => array(
          new OrderedArrayAttribute(
            new QuotaEntryAttribute(
              _('Quota informations for this user'),
              'quota',
              array(
                new SelectAttribute(
                  _('Device'), _('Device this quota is for'),
                  'quotaDevice', TRUE
                ),
                new UnitIntAttribute(
                  _('Block soft limit'), _(''),
                  'quotaBlockSoftLimit', TRUE,
                  $unit_options,
                  0 /*min*/, FALSE/*no max*/
                ),
                new UnitIntAttribute(
                  _('Block hard limit'), _(''),
                  'quotaBlockHardLimit', TRUE,
                  $unit_options,
                  0 /*min*/, FALSE/*no max*/
                ),
                new IntAttribute(
                  _('Inode soft limit'), _(''),
                  'quotaInodeSoftLimit', TRUE,
                  0 /*min*/, FALSE/*no max*/
                ),
                new IntAttribute(
                  _('Inode hard limit'), _(''),
                  'quotaInodeHardLimit', TRUE,
                  0 /*min*/, FALSE/*no max*/
                ),
                new SelectAttribute(
                  _('Server'), _('Server hosting the device this quota is for'),
                  'quotaServer', TRUE
                ),
              ),
              ':'
            ),
            FALSE,  // no order
            array(),
            TRUE    // edit button
          )
        )
      ),
    );
  }

  function __construct (&$config, $dn = NULL, $parent = NULL)
  {
    /* Get List of server with Quota service */
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=goServer)(objectClass=quotaService))', array('cn','description','quotaDeviceParameters'));
    $this->quotaDeviceParameters = array();
    $quotaServersList = array();
    while ($attrs = $ldap->fetch()) {
      $this->quotaDeviceParameters[$attrs['cn'][0]] = $attrs['quotaDeviceParameters'];
      unset($this->quotaDeviceParameters[$attrs['cn'][0]]['count']);
      $display = $attrs['cn'][0];
      if (isset($attrs['description'][0])) {
        $display .= ' - '.$attrs['description'][0];
      }
      $quotaServersList[$attrs['cn'][0]] = $display;
    }

    parent::__construct($config, $dn, $parent);

    $this->attributesAccess['quota']->setLinearRendering(FALSE);
    $this->attrQuotaServer =& $this->attributesAccess['quota']->attribute->attributes[5];
    $this->attrQuotaDevice =& $this->attributesAccess['quota']->attribute->attributes[0];

    $this->attrQuotaServer->setChoices(array_keys($quotaServersList), array_values($quotaServersList));
    $quotaDeviceList = $this->getDeviceList();
    $this->attrQuotaDevice->setChoices(array_keys($quotaDeviceList), array_values($quotaDeviceList));
  }

  function getDeviceList()
  {
    $quotaDeviceList = array();
    foreach ($this->quotaDeviceParameters[$this->attrQuotaServer->getValue()] as $infos) {
      $quotas = explode(":", $infos);
      $quotaDeviceList[$quotas[0]] = $quotas[0]." - ".$quotas[2];
    }
    return $quotaDeviceList;
  }

  function getBlockSize($server, $device)
  {
    foreach ($this->quotaDeviceParameters[$server] as $infos) {
      $quotas = explode(":", $infos);
      if ($quotas[0] == $device) {
        return $quotas[1];
      }
    }
    return FALSE;
  }
}

class quotaAccount extends plugin
{
  var $plHeadline     = "quota";
  var $plDescription  = "Plugin quota";

  var $quotaDeviceParameters  = array();
  var $quotaServersList       = array();
  var $quotaServer            = "";

  var $quotaDevice            = "";

  var $memberList             = array();

  var $quota_blocksoft        = 0;
  var $quota_blocksoft_unit   = 'K';
  var $quota_blockhard        = 0;
  var $quota_blockhard_unit   = 'K';
  var $quota_inodesoft        = 0;
  var $quota_inodehard        = 0;

  var $quota_subattributes = array( 'quota_blocksoft','quota_blocksoft_unit',
                                    'quota_blockhard','quota_blockhard_unit',
                                    'quota_inodesoft','quota_inodehard',
                                  );

  var $unit_options;

  var $view_logged            = FALSE;
  var $is_account             = FALSE;
  var $initially_was_account  = FALSE;

  var $quota = array();

  var $attributes = array("quota");
  var $objectclasses = array("systemQuotas");

  /* CLASS CONSTRUCTOR               */
  function quotaAccount(&$config, $dn= NULL)
  {
    global $class_mapping;

    /* loading global configuration of fusiondirectory */

    $this->config= $config;

    plugin::plugin($config,$dn);

    /* create a link for reading in LDAP tree */
    $ldap= $this->config->get_ldap_link();

    /* Check if DN isn't null */
    if ($dn !== NULL) {
      /* if DN exist , we check if quota clas is declared : show or not the quota screen */
      if (isset($this->attrs['objectClass']) && in_array ('quotaAccount', $this->attrs['objectClass'])) {
        $this->is_account = TRUE;
      }

      /* Save initial account state */
      $this->initially_was_account = $this->is_account;

      #fetch all information
      $tmp = array();
      if (isset($this->attrs['quota'])) {
        for ($i= 0; $i<$this->attrs['quota']['count']; $i++){
          $tmp[$this->attrs['quota'][$i]] = $this->attrs['quota'][$i];
        }
      }
      $this->quota = $tmp;
    }

    $this->unit_options = array('K'=>_('KB'),'M'=>_('MB'),'G'=>_('GB'),'T'=>_('TB'));

    /********************** SERVER SELECTION SYSTEM ***********************/
    $ldap = $this->config->get_ldap_link();

    /* Get List of server with Quota service */
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=goServer)(objectClass=quotaService))", array("cn", "description","quotaDeviceParameters"));
    while ($attrs = $ldap->fetch()) {
      $entry['cn'] = $attrs['cn'][0];
      if (isset($attrs['description'][0])) {
        $entry['description'] = $attrs['description'][0];
      } else {
        $entry['description'] = "";
      }
      $quotaDeviceParameters = array();
      for ($i= 0; $i<$attrs['quotaDeviceParameters']['count']; $i++) {
        $quotaDeviceParameters[] = $attrs['quotaDeviceParameters'][$i];
      }
      $this->quotaDeviceParameters[$entry['cn']] = $quotaDeviceParameters;
      $server_display = $entry['cn'];
      if (!empty($entry['description'])) {
        $server_display .= " - ".$entry['description'];
      }
      $this->quotaServersList[$entry['cn']] = $server_display;
    }
    if (!empty($this->quotaServersList)) {
      $this->quotaServer = key($this->quotaServersList);
    }
  }

  function execute()
  {

    /* Call parent execute */
    plugin::execute();
    $display= "";

    /****************
      Account status
     ****************/

    if (empty($this->quotaServersList)) {
      $display = "<img alt=\"\" src=\"images/small-error.png\" align=\"middle\">&nbsp;<b>".
                _("There is no quota server configured")."</b>";
      $display .= back_to_main();
      return $display;
    }

    /* Templates now! */
    $smarty = get_smarty();

    $smarty->assign("quotaServersList",$this->quotaServersList);
    $smarty->assign("quotaServer",$this->quotaServer);

    $quotaDeviceList = array();
    for ($i=0; $i<count($this->quotaDeviceParameters[$this->quotaServer]); $i++) {
      $quotas = explode(":",$this->quotaDeviceParameters[$this->quotaServer][$i]);
      $quotaDeviceList[$quotas[0].":".$quotas[1]] = $quotas[0]." - ".$quotas[2];
    }

    if (empty($this->quotaDevice)) {
      $this->quotaDevice = key($quotaDeviceList);
    }

    /* quotaDeviceList is the list of available value that could be computed */
    $smarty->assign("quotaDeviceList",$quotaDeviceList);
    $smarty->assign("quotaDevice",$this->quotaDevice);

    /******************** DISPLAY LIST OF QUOTA WITH EDIT/DELETE ICON *********************/
    /* create list box */
    $QuotaList = new divSelectBox("quota");
    /* set listbox height */
    $QuotaList->SetHeight(56);
    /* store in php value html code for editing entries by POST variable */
    $link_delete = "<a href='?plug=".$_GET['plug']."&act=delete&id=%s'><img style='border: 0px' src='images/lists/trash.png'></a>";

    /* walk all values in quota object */
    foreach ($this->quota as $quota => $values ) {
      /* fetch all values for all record in object */
      $quota_exploded = explode(":",$values);
      /* fetch the blocksize following the server and device used */
      for ($i=0; $i<count($this->quotaDeviceParameters[$quota_exploded[5]]); $i++) {
        $quota_available=explode(":",$this->quotaDeviceParameters[$quota_exploded[5]][$i]);
        if ($quota_available[0] == $quota_exploded[0]) {
          $blocksize = $quota_available[1];
          break;
        }
      }
      /* convert value */
      $warning_size   = $this->ldap_to_human_size($quota_exploded[1],$blocksize);
      $critical_size  = $this->ldap_to_human_size($quota_exploded[2],$blocksize);

      /* create entries for list box */
      $QuotaList->AddEntry(array(
        array("string" => _("Server")." : " . $quota_exploded[5]),
        array("string" => _("Device")." : " . $quota_exploded[0]),
        array("string" => _("Block soft limit")." : ".$warning_size),
        array("string" => _("Block hard limit")." : ".$critical_size),
        array("string" => _("Inode soft limit")." : ".$quota_exploded[3]),
        array("string" => _("Inode hard limit")." : ".$quota_exploded[4]),
        array("string" => str_replace("%s",base64_encode($quota),$link_delete))
      ));
    }

    foreach ($this->quota_subattributes as $attr) {
      $smarty->assign("$attr",$this->$attr);
    }

    $smarty->assign("unit_options",$this->unit_options);

    /* Display template */
    $smarty->assign("QuotaList",$QuotaList->DrawList());
    $display .= $smarty->fetch(get_template_path('quota.tpl', TRUE, dirname(__FILE__)));
    return $display;
  }

  function save_object()
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,"", "save_object");
    /******************** PRECISE DELETE QUOTA ACTION *****************************/
    /* check posts for delete zone */
    /* the id is the value stored but base64coded */
    if (isset($_GET['act']) && ($_GET['act'] == "delete")) {
      $tmp = base64_decode($_GET['id']);
      unset($this->quota[$tmp]);
    }

    if (isset($_POST['quotaTab'])) {
      plugin::save_object();

      foreach ($this->quota_subattributes as $attr) {
        if (isset($_POST["$attr"])) {
          if (in_array($attr,array('quota_blocksoft_unit','quota_blockhard_unit'))) {
            if (isset($this->unit_options[$_POST["$attr"]])) {
              $this->$attr = $_POST["$attr"];
            }
          } else {
            $this->$attr = intval($_POST["$attr"]);
          }
        }
      }

      if (isset($_POST['quotaServer']) && ($this->quotaServer != $_POST['quotaServer'])) {
        $this->quotaServer = $_POST['quotaServer'];
        unset($this->quotaDevice);
      }
      /****************** ADD A NEW QUOTA ***************************/
      elseif ((isset($_POST['addquota'])) &&
              $this->acl_is_writeable("quota")) {
        $quotaUsed    = explode(":",$this->quotaDevice);
        $quota_device = $quotaUsed[0];
        $blocksize    = $quotaUsed[1];

        $units = array(
                        'K' => 1024,
                        'M' => 1024 * 1024,
                        'G' => 1024 * 1024 * 1024,
                        'T' => 1024 * 1024 * 1024 * 1024,
                      );

        # convert human quantity on computer value
        $quota_blocksoft = ceil($this->quota_blocksoft * $units[$this->quota_blocksoft_unit] / $blocksize);

        $quota_blockhard = ceil($this->quota_blockhard * $units[$this->quota_blockhard_unit] / $blocksize);

        if ($quota_blockhard < $quota_blocksoft) {
          $quota_blockhard = $quota_blocksoft;
        }
        if ($this->quota_inodehard < $this->quota_inodesoft) {
          $this->quota_inodehard = $this->quota_inodesoft;
        }

        $quota_added =  $quota_device.":".$quota_blocksoft.":".$quota_blockhard.":".
                        $this->quota_inodesoft.":".$this->quota_inodehard.":".
                        $this->quotaServer;

        /* we add value on object */
        $this->quota[$quota_added] = $quota_added;
      }
    }
  }

  static function plInfo()
  {
    return array(
      "plShortName"   => _("Quota"),
      "plDescription" => _("Plugin for quota support"),
      "plSelfModify"  => TRUE,
      "plPriority"    => 5,
      "plCategory"    => array("users"),
      "plObjectType"  => array("user"),

      "plProvidedAcls" => array(
        "uid"   =>  _("uid"),
        "quota" =>  _("quota"),
      )
    );
  }

  static function ldap_to_human_size($blocknumber, $blocksize) {
    $size = $blocknumber * $blocksize;

    $mod = 1024;

    $units = array(_('B'),_('KB'),_('MB'),_('GB'),_('TB'),_('PB'),_('EB'));
    $i = 0;
    while ($size >= $mod) {
        $size /= $mod;
        $i++;
    }
    return round($size, 2).' '.$units[$i];
  }
}


// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
