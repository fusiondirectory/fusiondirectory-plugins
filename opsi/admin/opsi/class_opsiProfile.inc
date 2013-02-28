<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013 FusionDirectory

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

class SubmittingOPSIServerAttribute extends OPSIServerAttribute
{
  function __construct ($label, $description, $ldapName, $required = FALSE, $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, array(), "", NULL, $acl);
    $this->setSubmitForm(TRUE);
  }

  function setValue($value)
  {
    $v = $this->getValue();
    parent::setValue($value);
    if (is_object($this->plugin) && ($v != $this->getValue())) {
      $this->plugin->server_changed();
    }
  }
}

class opsiProfile extends simplePlugin
{
  var $mainTab = TRUE;

  var $objectclasses   = array('opsiProfile');

  var $macAddress = NULL;

  var $products;

  public static function plInfo()
  {
    return array(
      'plShortName'   => _('OPSI profile'),
      'plDescription' => _('OPSI profile'),
      'plSelfModify'  => FALSE,
      'plCategory'    => array('opsiProfile' => array('description'  => _('OPSI profile'),
                                                          'objectClass'  => 'opsiProfile')),
      'plObjectType'  => array('opsiProfile' => array( 'name'  => _('OPSI profile'),
                                                          'filter'  => 'objectClass=opsiProfile')),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Name'),
        'attrs' => array(
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
        'name'  => _('Entries'),
        'attrs' => array(
          new SelectAttribute(
            _('Netboot product'), _('The netboot product to use for this profile'),
            'fdOpsiNetbootProduct', FALSE
          ),
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('The localboot products to install for this profile'),
              'fdOpsiLocalbootProduct',
              array(
                new SelectAttribute(
                  '', '',
                  'fdOpsiLocalbootProduct_product', TRUE
                ),
                new SelectAttribute(
                  '', '',
                  'fdOpsiLocalbootProduct_action', TRUE,
                  array('setup', 'always', 'once', 'custom', 'userlogin')
                )
              ),
              '|','',_('Localboot products')
            ),
            FALSE, // non-ordered
            array(),
            TRUE // edition
          ),
        )
      ),
    );
  }

  public function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->attributesAccess['cn']->setUnique(TRUE);
  }

  public function server_changed ()
  {
    if (!isset($this->products[$this->fdOpsiServerDN])) {
      $s_daemon = new supportDaemon();
      if (!$s_daemon->is_available()) {
        msg_dialog::display(
          _("Could not contact argonaut server"),
          msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
        );
        return;
      }
      $ldap = $this->config->get_ldap_link();
      $ldap->cat($this->fdOpsiServerDN);
      if ($attrs = $ldap->fetch()) {
        $this->macAddress = $attrs['macAddress'][0];
      } else {
        msg_dialog::display(
          sprintf_(_("Could not find %s in the LDAP"),$this->fdOpsiServerDN),
          msgPool::ldaperror($ldap->get_error()), ERROR_DIALOG
        );
        return FALSE;
      }
      $netboots = $s_daemon->append_call('OPSI.get_netboots', array($this->macAddress), array('args' => array(array('id','name'))));
      if ($s_daemon->is_error()) {
        msg_dialog::display(
          _('Could contact OPSI server'),
          msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
        );
        return;
      }
      $this->products[$this->fdOpsiServerDN]['netboots'] = $netboots;
      $localboots = $s_daemon->append_call('OPSI.get_localboots', array($this->macAddress), array('args' => array(array('id','name'))));
      if ($s_daemon->is_error()) {
        msg_dialog::display(
          _('Could contact OPSI server'),
          msgPool::siError($s_daemon->get_error()), ERROR_DIALOG
        );
        return;
      }
      $this->products[$this->fdOpsiServerDN]['localboots'] = $localboots;
    }
    $choices_values = array();
    $choices_labels = array();
    foreach ($this->products[$this->fdOpsiServerDN]['netboots'] as $netboot) {
      $choices_values[] = $netboot['id'];
      $choices_labels[] = $netboot['name'];
    }
    $this->attributesAccess['fdOpsiNetbootProduct']->setChoices($choices_values, $choices_labels);
    $choices_values = array();
    $choices_labels = array();
    foreach ($this->products[$this->fdOpsiServerDN]['localboots'] as $localboot) {
      $choices_values[] = $localboot['id'];
      $choices_labels[] = $localboot['name'];
    }
    $this->attributesAccess['fdOpsiLocalbootProduct']->attribute->attributes[0]->setChoices($choices_values, $choices_labels);
  }
}
?>
