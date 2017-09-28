<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2016 FusionDirectory

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

class opsiSoftwareList extends simplePlugin
{
  var $mainTab = TRUE;
  var $objectclasses   = array('opsiSoftwareList');

  var $macAddress = NULL;
  var $products;

  public static function plInfo()
  {
    return array(
      'plShortName'   => _('OPSI list'),
      'plDescription' => _('OPSI software list'),
      'plIcon'        => 'geticon.php?context=applications&icon=opsi-software-list&size=16',
      'plSelfModify'  => FALSE,
      'plObjectType'  => array('opsiSoftwareList' => array(
        'name'    => _('OPSI software list'),
        'filter'  => 'objectClass=opsiSoftwareList',
        'ou'      => get_ou('opsiRDN'),
        'icon'    => 'geticon.php?context=applications&icon=opsi-software-list&size=16',
      )),

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Infos'),
        'attrs' => array(
          new BaseSelectorAttribute(get_ou('opsiRDN')),
          new SubmittingOPSIServerAttribute(
            _('OPSI server'), _('OPSI server to use for deployment'),
            'fdOpsiServerDN', TRUE
          ),
          new HostNameAttribute(
            _('Name'), _('Name of this OPSI profile'),
            'cn', TRUE
          ),
        )
      ),
      'products' => array(
        'name'  => _('Softwares'),
        'attrs' => array(
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('The localboot products to install with this list'),
              'fdOpsiLocalbootProduct',
              array(
                new SelectAttribute(
                  '', '',
                  'fdOpsiLocalbootProduct_product', TRUE
                ),
                new SelectAttribute(
                  '', '',
                  'fdOpsiLocalbootProduct_action', TRUE,
                  array('setup', 'always', 'once', 'custom', 'userlogin', 'update')
                )
              ),
              '|', '', _('Localboot products')
            ),
            FALSE, // non-ordered
            array(),
            TRUE // edition
          ),
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['cn']->setUnique(TRUE);
    $this->attributesAccess['fdOpsiLocalbootProduct']->attribute->attributes[0]->setSubmitForm('product_changed');
  }

  public function server_changed ($opsi_args = NULL)
  {
    global $config;
    if ($opsi_args == NULL) {
      $opsi_args = array('id','name','setupScript','alwaysScript','onceScript','customScript','userLoginScript','updateScript','productVersion','packageVersion');
    }
    if (!isset($this->products[$this->fdOpsiServerDN])) {
      $s_daemon = new supportDaemon();
      if (!$s_daemon->is_available()) {
        msg_dialog::display(
          _("Could not contact argonaut server"),
          msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
        );
        return;
      }
      $ldap = $config->get_ldap_link();
      $ldap->cat($this->fdOpsiServerDN);
      if ($attrs = $ldap->fetch()) {
        if (isset($attrs['macAddress'])) {
          $this->macAddress = $attrs['macAddress'][0];
        } else {
          msg_dialog::display(
            _("No mac address"),
            sprintf(_("Server %s has no mac address configured in the LDAP"), $this->fdOpsiServerDN), ERROR_DIALOG
          );
          return FALSE;
        }
      } else {
        msg_dialog::display(
          sprintf(_("Could not find %s in the LDAP"), $this->fdOpsiServerDN),
          msgPool::ldaperror($ldap->get_error()), ERROR_DIALOG
        );
        return FALSE;
      }
      $localboots = $s_daemon->append_call('OPSI.get_localboots', array($this->macAddress), array('args' => array($opsi_args)));
      if ($s_daemon->is_error()) {
        msg_dialog::display(
          _('Failed to contact OPSI server'),
          msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
        );
        return;
      }
      $this->products[$this->fdOpsiServerDN]['localboots'] = array();
      foreach ($localboots as $localboot) {
        $this->products[$this->fdOpsiServerDN]['localboots'][$localboot['id']] = $localboot;
      }
    }
    $choices = array();
    foreach ($this->products[$this->fdOpsiServerDN]['localboots'] as $id => $infos) {
      $choices[$id] = sprintf(_('%s (%s-%s)'), $infos['id'], $infos['productVersion'], $infos['packageVersion']);
    }
    $this->attributesAccess['fdOpsiLocalbootProduct']->attribute->attributes[0]->setChoices(
      array_keys($choices),
      array_values($choices)
    );
  }

  public function product_changed ()
  {
    $localboot = $this->attributesAccess['fdOpsiLocalbootProduct']->attribute->attributes[0]->getValue();

    $actions = array();
    foreach (array('setup', 'always', 'once', 'custom', 'userLogin', 'update') as $action) {
      if (!empty($this->products[$this->fdOpsiServerDN]['localboots'][$localboot][$action.'Script'])) {
        $actions[] = strtolower($action);
      }
    }
    $this->attributesAccess['fdOpsiLocalbootProduct']->attribute->attributes[1]->setChoices($actions);
  }
}
?>
