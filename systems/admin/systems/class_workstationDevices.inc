<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2012  FusionDirectory

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

class workstationDevices extends simplePlugin
{
  /* Needed values and lists */
  var $ignore_account = TRUE;

  var $objectclasses = array("GOhard");

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array(
      'keyboard' => array(
        'icon'  => 'plugins/systems/images/keyboard.png',
        'name'  => _('Keyboard'),
        'attrs' => array(
          new SelectAttribute (
            _('Model'), _('The keyboard model'),
            'gotoXKbModel', TRUE,
            array(
              "btc9000", "chicony", "compaq", "dell", "dell101", "everex",
              "flexpro", "geniuscomfy", "hp", "itouch", "jp106", "logicordless",
              "logiinetnav", "logiinternet", "macintosh", "microsoft",
              "microsoftpro", "omnikey101", "pc101", "pc102", "pc104",
              "pc105", "rapidaccess", "rapidaccess2", "winbook"
            )
          ),
          new SelectAttribute (
            _('Layout'), _('The keyboard layout'),
            'gotoXKbLayout', TRUE,
            array("de", "en", "es", "us", "fr")
          ),
          new SelectAttribute (
            _('Variant'), _('The keyboard variant'),
            'gotoXKbVariant', TRUE,
            array ("nodeadkeys", "basic")
          ),
        )
      ),
      'mouse' => array(
        'icon'  => 'plugins/systems/images/mouse.png',
        'name'  => _('Mouse'),
        'attrs' => array(
          new SelectAttribute (
            _('Type'), _('Mouse type'),
            'gotoXMouseType', TRUE,
            array("auto", "explorerps/2", "ImPS/2", "PS/2", "Microsoft", "Logitech")
          ),
          new SelectAttribute (
            _('Port'), _('Mouse port'),
            'gotoXMouseport', TRUE,
            array("/dev/input/mice", "/dev/mouse", "/dev/psaux", "/dev/ttyS0", "/dev/ttyS1")
          ),
        )
      ),
      'phone' => array(
        'icon'  => 'plugins/systems/images/select_phone.png',
        'name'  => _('Phone hardware'),
        'attrs' => array(
          new SelectAttribute (
            _('Telephone'), _('Telephone'),
            'goFonHardware', TRUE
          ),
        )
      ),
      'graphic' => array(
        'icon'  => 'plugins/systems/images/hardware.png',
        'name'  => _('Graphic device'),
        'attrs' => array(
          new SelectAttribute (
            _('Driver'), _('Graphic driver'),
            'gotoXDriver', TRUE
          ),
          new SelectAttribute (
            _('Resolution'), _('Graphic resolution'),
            'gotoXResolution', TRUE,
            array(
              "640x480"   => "640x480",
              "800x600"   => "800x600",
              "1024x768"  => "1024x768",
              "1152x864"  => "1152x864",
              "1280x1024" => "1280x1024",
              "1400x1050" => "1400x1050",
              "1600x1200" => "1600x1200"
            )
          ),
          new SelectAttribute (
            _('Color depth'), _('Color depth'),
            'gotoXColordepth', TRUE,
            array("8", "15", "16", "24"),
            '',
            array("8 "._("bit"), "15 "._("bit"), "16 "._("bit"), "24 "._("bit"))
          ),
        )
      ),
      'display' => array(
        'icon'  => 'plugins/systems/images/display.png',
        'name'  => _('Display device'),
        'attrs' => array(
          new StringAttribute (
            _('Type'), _('Monitor type'),
            'gotoXMonitor', FALSE
          ),
          new BooleanAttribute (
            _('Use DDC for automatic detection'), _('Use DDC for automatic detection'),
            'useDDC', FALSE,
            TRUE
          ),
          new StringAttribute (
            _('HSync'), _('Horizontal sync (kHz)'),
            'gotoXHsync', TRUE,
            '30+55', '', '/^\d+(\.\d+)?([+-]\d+(\.\d+)?)?$/', '30-55'
          ),
          new StringAttribute (
            _('VSync'), _('Vertical sync (Hz)'),
            'gotoXVsync', TRUE,
            '50+70', '', '/^\d+(\.\d+)?([+-]\d+(\.\d+)?)?$/', '50-70'
          ),
        )
      ),
      'scan' => array(
        'icon'  => 'plugins/systems/images/scanner.png',
        'name'  => _('Scan device'),
        'attrs' => array(
          new BooleanAttribute (
            _('Provide scan services'), _('Provide scan services'),
            'gotoScannerEnable'
          ),
        )
      ),
    );
  }

  function __construct (&$config, $dn = NULL, $parent = NULL)
  {
    parent::__construct ($config, $dn, $parent);

    $this->attributesInfo['display']['attrs']['gotoXMonitor']->setDisabled(TRUE);

    $this->attributesInfo['display']['attrs']['useDDC']->setInLdap(FALSE);
    $this->attributesInfo['display']['attrs']['useDDC']->setManagedAttributes(
      array(
        'disable' => array(
          TRUE => array(
            'display' => array('gotoXHsync', 'gotoXVsync'),
          )
        )
      )
    );

    if ($this->config->get_cfg_value("resolutions") != "") {
      $resolutions = $this->attributesInfo['graphic']['attrs']['gotoXResolution']->getChoices();
      $file = $this->config->get_cfg_value("resolutions");

      if (is_readable($file)) {
        $str = file_get_contents($file);
        $lines = preg_split("/\n/", $str);
        foreach ($lines as $line) {
          $line = trim($line);
          if (!empty($line)) {
            $resolutions[] = $line;
          }
        }
      } else {
        msg_dialog::display(_("Configuration error"), msgPool::cannotReadFile($file), WARNING_DIALOG);
      }

      $this->attributesInfo['graphic']['attrs']['gotoXResolution']->setChoices(array_unique($resolutions));
    }

    /* Get list of available xdrivers */
    $XDrivers = $this->getListOfXDrivers();
    array_unshift($XDrivers, "["._("unknown")."]");
    $this->attributesInfo['graphic']['attrs']['gotoXDriver']->setChoices($XDrivers);

    /* try to read additional keyboard layouts
     */
    if (file_exists(SYSTEMS_DIR."/keyboardLayouts")) {
      if (is_readable(SYSTEMS_DIR."/keyboardLayouts")) {
        $choices = $this->attributesInfo['keyboard']['attrs']['gotoXKbLayout']->getChoices();
        $output = $choices;
        $str = file_get_contents(SYSTEMS_DIR."/keyboardLayouts");
        $tmp = preg_split("/\n/", $str);
        foreach ($tmp as $entry) {
          if ((!empty($entry)) && (!preg_match("/^#/", $entry))) {
            $entry = trim($entry);
            $tmp2 = explode(":", $entry);
            $la = trim($tmp2[0]);   // What would be saved to ldap
            $da = trim($tmp2[1]);   // This wis displayed in the listbox
            $choices[]  = $la;
            $output[]   = $da;
          }
        }
        $this->attributesInfo['keyboard']['attrs']['gotoXKbLayout']->setChoices($choices, $output);
      }
    }

    /* Load phone hardware list
     */
    $hardware_list = array();
    $tmp = get_sub_list("(objectClass=goFonHardware)", "", array(get_ou("phoneRDN")),
                  $this->config->current['BASE'], array("cn","description"), GL_NO_ACL_CHECK);
    foreach ($tmp as $attrs) {
      $cn           = $attrs['cn'][0];
      $description  = "";
      if (isset($attrs['description'])) {
        $description = " - ".$attrs['description'][0];
      }
      $hardware_list[$cn] = "$cn$description";
    }
    $hardware_list["automatic"] = _("automatic");
    ksort($hardware_list);

    /* These departments may contain objects that have
        goFonHardware set.
     */
    //~ $deps_a = array(
      //~ get_people_ou(),
      //~ get_ou("ogroupRDN"),
      //~ get_ou("serverRDN"),
      //~ get_ou("terminalRDN"),
      //~ get_ou("workstationRDN"),
      //~ get_ou("printerRDN"),
      //~ get_ou("componentRDN"),
      //~ get_ou("phoneRDN")
    //~ );

    //~ $tmp = get_sub_list("(goFonHardware=*)", "", $deps_a, $this->config->current['BASE'],
        //~ array('cn','dn','goFonHardware'), GL_NO_ACL_CHECK);
    //~ $used_hardware = array();
    //~ foreach ($tmp as $attrs) {
      //~ $cn = $attrs['goFonHardware'][0];
      //~ if (isset($hardware_list[$cn])) {
        //~ $used_hardware[$cn] = $cn;
      //~ }
    //~ }
    $this->attributesInfo['phone']['attrs']['goFonHardware']->setChoices(array_values($hardware_list));

    if (preg_match("/\+/",$this->attributesInfo['display']['attrs']['gotoXHsync']->getValue())) {
      $this->attributesInfo['display']['attrs']['useDDC']->setValue(TRUE);
      $this->attributesInfo['display']['attrs']['gotoXHsync']->setValue(
        preg_replace(
          "/\+/","-",
          $this->attributesInfo['display']['attrs']['gotoXHsync']->getValue()
        )
      );
      $this->attributesInfo['display']['attrs']['gotoXVsync']->setValue(
        preg_replace(
          "/\+/","-",
          $this->attributesInfo['display']['attrs']['gotoXVsync']->getValue()
        )
      );
    }

    /* Workaround to fill in inherited values if we've specified an objectclass */
    //~ $SelectedSystemType = session::get("SelectedSystemType");
    //~ if (isset($SelectedSystemType['ogroup']) && $SelectedSystemType['ogroup'] != 'none'){
      //~ $this->XResolutions= array('' => _("inherited"));
      //~ $this->XColordepths= array('' => _("inherited"));
      //~ $this->XKbModels= array('' => _("inherited"));
      //~ $this->XKbLayouts= array('' => _("inherited"));
      //~ $this->XKbVariants= array('' => _("inherited"));
      //~ $this->MouseTypes= array('' => _("inherited"));
      //~ $this->MousePorts= array('' => _("inherited"));
    //~ }
  }

  function save_object()
  {
    parent::save_object();
    if ($this->attributesInfo['display']['attrs']['useDDC']->getValue()) {
      $this->attributesInfo['display']['attrs']['gotoXHsync']->setValue("30+55");
      $this->attributesInfo['display']['attrs']['gotoXVsync']->setValue("50+70");
    }
  }

  /* Save to LDAP */
  function save()
  {
    /* remove objectclass GOhard if this is an ogroup tab */
    if (isset($this->parent->by_object['ogroup'])) {
      $this->objectclasses = array();
    }

    parent::save();

    /* Send goto reload event to supportDaemon */
    $this->send_goto_reload();
  }


  function getListOfXDrivers()
  {
    /* Generate a list of xdrivers from SYSTEMS_DIR./xdrivers */
    $drivers = array();
    if (file_exists(SYSTEMS_DIR.'/xdrivers')) {
      $xdrivers = file (SYSTEMS_DIR.'/xdrivers');
      foreach ($xdrivers as $line) {
        if (!preg_match ("/^#/", $line)) {
          $drivers[] = trim($line);
        }
      }
    } else {
      $drivers = array(
        "ati", "atimisc", "chips", "cirrus", "cyrix", "fbdev", "fglrx",
        "i128", "i740", "i810", "intel", "imstt", "mga", "neomagic", "newport", "nouveau", "nsc",
        "nv", "nvidia", "r128", "radeonhd", "radeon", "rendition", "s3", "s3virge", "savage",
        "siliconmotion", "sis", "tdfx", "tga", "trident", "tseng", "vboxvideo", "vesa", "vga", "vmware"
      );
    }
    $tmp = array();
    foreach ($drivers as $name) {
      $tmp[$name] = $name;
    }

    return array_values($tmp);
  }


  /*! \brief  Send goto_reload event to support daemon
   */
  function send_goto_reload()
  {
    /* this code was removed */
  }

  function execute ()
  {
    return "<style type=\"text/css\">
      .plugin_section, .plugin_section:nth-child(even) {
        clear:none;
        display:table-cell;
        float:none;
        width:29%;
        margin-left:0.5%;
        margin-right:0.5%;
      }
    </style>".parent::execute();
  }

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Devices"),
      "plDescription" => _("Workstation devices"),
      "plSelfModify"  => FALSE,
      "plPriority"    => 10,
      "plCategory"    => array("workstation", "ogroups"),
      "plObjectType"  => array("workstation", "ogroup"),

      "plProvidedAcls" => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }
}

?>
