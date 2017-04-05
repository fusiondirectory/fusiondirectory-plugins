<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2016  FusionDirectory

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

class webLink extends simplePlugin
{
  var $objectclasses  = array('webLink');
  var $displayHeader  = TRUE;

  var $inheritance = array('gosaGroupOfNames' => 'member');

  static function plInfo ()
  {
    return array(
      'plShortName'     => _('Web link'),
      'plDescription'   => _('Edit web link'),
      'plSelfModify'    => FALSE,
      'plObjectType'    => array('workstation','server','ipmi','component','printer'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Web link'),
        'attrs' => array(
          new SelectAttribute (
            _('Protocol'), _('Protocol to use to access this computer Web page'),
            'fdWebLinkProtocol', TRUE,
            array('https', 'http')
          ),
          new DisplayAttribute (
            _('Link'), _('Web link to this computer'),
            'fdWebLink', FALSE
          )
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    $this->attributesAccess['fdWebLinkProtocol']->setSubmitForm(TRUE);
    $this->attributesAccess['fdWebLink']->setAllowHTML(TRUE);
  }

  function renderAttributes($readOnly = FALSE)
  {
    $link = $this->computeLink();
    $this->fdWebLink = '<a target="_blank" href="'.$link.'">'.$link.'</a>';
    return parent::renderAttributes($readOnly);
  }

  function computeLink()
  {
    global $config;
    if ($this->member_of_group) {
      $protocol = $this->group_attrs['fdWebLinkProtocol'][0];
    } else {
      $protocol = $this->fdWebLinkProtocol;
    }
    $basetab  = $this->parent->getBaseObject();
    $hosts    = $basetab->ipHostNumber;
    if (!is_array($hosts)) {
      $hosts = array($hosts);
    }
    if (isset($this->parent->by_object['dnsHost']) && ($this->parent->by_object['dnsHost']->is_account)) {
      $zones  = $this->parent->by_object['dnsHost']->fdDNSZoneDn;
      $ldap   = $config->get_ldap_link();
      foreach ($zones as $zoneDn) {
        $ldap->cat($zoneDn);
        if ($attrs = $ldap->fetch()) {
          $hosts[] = $basetab->cn.'.'.preg_replace('/\.$/', '', $attrs['zoneName'][0]);
        }
      }
    }
    return $protocol.'://'.end($hosts);
  }
}

?>
