<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2014  FusionDirectory

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

class printGeneric extends simplePlugin
{
  var $mainTab = TRUE;

  var $netConfigDNS;

  var $objectclasses  = array('top', 'gotoPrinter');

  var $preInitAttributes = array('networkSettings');

  /* Return plugin informations */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Printer'),
      'plDescription' => _('Print generic'),
      'plPriority'    => 10,
      'plObjectType'  => array(
        'printer' => array(
          'name'        => _('Printer'),
          'description' => _('Printer'),
          'filter'      => 'objectClass=gotoPrinter',
          'icon'        => 'geticon.php?context=devices&icon=printer&size=16',
          'ou'          => get_ou('printerRDN'),
        )
      ),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Properties'),
        'attrs' => array(
          new BaseSelectorAttribute(get_ou('printerRDN')),
          new HostNameAttribute (
            _('Name'), _('The name of the printer'),
            'cn', TRUE
          ),
          new StringAttribute (
            _('Description'), _('A short description of the printer'),
            'description', FALSE
          ),
        )
      ),
      'details' => array(
        'name'  => _('Details'),
        'attrs' => array(
          new StringAttribute (
            _('Printer location'), _('The location of the printer'),
            'l', FALSE
          ),
          new StringAttribute (
            _('Printer URL'), _('The URL of the printer'),
            'labeledURI', TRUE
          ),
        )
      ),
      'network' => array(
        'name'      => _('Network settings'),
        'attrs'     => array(new NetworkSettingsAttribute()),
        'template'  => get_template_path('network_section.tpl', TRUE, dirname(__FILE__)),
        'icon'      => 'geticon.php?context=categories&icon=applications-internet&size=16',
      ),
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->attributesAccess['cn']->setUnique(TRUE);
    $this->netConfigDNS->MACisMust = FALSE;
  }

  function resetCopyInfos()
  {
    parent::resetCopyInfos();
    $this->netConfigDNS->resetCopyInfos();
  }

  function set_acl_base($base)
  {
    parent::set_acl_base($base);
    $this->netConfigDNS->set_acl_base($base);
  }

  function set_acl_category($cat)
  {
    parent::set_acl_category($cat);
    $this->netConfigDNS->set_acl_category($cat);
  }

  function remove_from_parent()
  {
    if (!$this->initially_was_account) {
      return;
    }

    if ($this->acl_is_removeable()) {
      $this->netConfigDNS->remove_from_parent();

      parent::remove_from_parent();
    }
  }

  protected function ldap_save($cleanup = TRUE)
  {
    parent::ldap_save($cleanup);
    $this->netConfigDNS->cn = $this->cn;
    $this->netConfigDNS->save();
  }

  function is_modal_dialog()
  {
    return (isset($this->netConfigDNS->dialog) && $this->netConfigDNS->dialog);
  }
}
?>
