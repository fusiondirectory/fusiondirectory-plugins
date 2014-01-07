<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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

class SubNodesAttribute extends OrderedArrayAttribute
{
  protected $objectClass;

  function __construct ($label, $description, $ldapName, $objectClass, $attributes, $values = array(), $edit_enabled = FALSE, $acl = "")
  {
    $attributes_keys = array();
    foreach ($attributes as $attribute) {
      $attributes_keys[$attribute->getLdapName()] = $attribute;
    }
    $composite = new CompositeAttribute(
      $description, $ldapName,
      $attributes_keys,
      FALSE, FALSE,
      $acl, $label
    );
    parent::__construct($composite, FALSE, $values, $edit_enabled);
    $this->objectClass = $objectClass;
  }

  protected function loadAttrValue ($attrs)
  {
    /* Should we take dn from attrs or plugin? */
    if (isset($attrs['dn'])) {
      $ldap = $this->plugin->config->get_ldap_link();
      $ldap->ls('objectClass='.$this->objectClass, $attrs['dn']);
      $this->value = array();
      while ($subattrs = $ldap->fetch()) {
        $this->attribute->resetToDefault();
        foreach ($this->attribute->attributes as &$attribute) {
          $attribute->loadAttrValue($subattrs);
        }
        unset($attribute);
        $this->value[] = $this->attribute->getValue();
      }
    } else {
      $this->resetToDefault();
    }
  }

  /* Not saving anything into base node */
  function fillLdapValue (&$attrs)
  {
    /* Remove crap made by plugin */
    unset ($attrs[$this->getLdapName()]);
  }

  /* Special LDAP treatment that this attribute does after plugin ldap save */
  function postLdapSave ($ldap)
  {
    /* First delete all old nodes */
    $ldap->ls('objectClass='.$this->objectClass, $this->plugin->dn, array('dn'));
    $delete = array();
    while ($attrs = $ldap->fetch()) {
      $delete[] = $attrs['dn'];
    }
    foreach ($delete as $dn) {
      $ldap->rmdir($dn);
    }
    /* Then add our values */
    foreach ($this->value as $val) {
      $attrs = array('objectClass' => array($this->objectClass));
      $this->attribute->setValue($val);
      foreach ($this->attribute->attributes as &$attribute) {
        $attribute->fillLdapValue($attrs);
      }
      unset($attribute);
      $dn = $this->compute_attribute_dn();
      $ldap->cd($dn);
      $ldap->add($attrs);
      if (!$ldap->success()) {
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $dn, LDAP_ADD, get_class()), LDAP_ERROR);
      }
    }
  }

  function compute_attribute_dn ()
  {
    /* Later we might want to be able to choose which attribute to use in the dn */
    reset($this->attribute->attributes);
    $attribute = key($this->attribute->attributes);
    return $attribute.'='.$this->attribute->attributes[$attribute]->computeLdapValue().','.$this->plugin->dn;
  }
}

class servicePostfix extends simpleService
{
  var $objectclasses = array('fdPostfixServer');

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Postfix (SMTP)'),
      'plDescription' => _('Postfix (SMTP)').' ('._('Services').')',
      'plIcon'        => 'plugins/mail/images/iconMiniSmtp.png',

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  /*!
  *  \brief The main function : information about attributes
  */
  static function getAttributesInfo ()
  {
    return array (
      'section1' => array (
        'name'  => _('Generic'),
        'icon' => 'images/rightarrow.png',
        'attrs' => array (
          new StringAttribute (
            _('Hostname'), _('The host name.'),
            'postfixMyHostname', TRUE,
            '', '', '/^([^\.]+\.)*[^\.]+$/', 'smtp'
          ),
          new StringAttribute (
            _('Domain'), _('The domain.'),
            'postfixMyDomain', TRUE,
            '', '', '/^([^\.]+\.)*[^\.]+$/', 'example.org'
          ),
          new IntAttribute (
            _('Max mail header size (KB)'), _('This value specifies the maximal header size.'),
            'postfixHeaderSizeLimit', FALSE,
            FALSE, FALSE, 0
          ),
          new IntAttribute (
            _('Max mailbox size (KB)'), _('Defines the maximal size of mail box.'),
            'postfixMailboxSizeLimit', FALSE,
            FALSE, FALSE, 0
          ),
          new IntAttribute (
            _('Max message size (KB)'), _('Specify the maximal size of a message.'),
            'postfixMessageSizeLimit', FALSE,
            FALSE, FALSE, 0
          )
        )
      ),
      'routing' => array (
        'name' => _('Domains and routing'),
        'attrs' => array (
          new SetAttribute (
            new StringAttribute (
              _('Domains to accept mail for'), _('Postfix networks'),
              'postfixMyDestinations'
            )
          ),
          new SubNodesAttribute (
            _('Transport table'), _('Transport table'),
            'fdPostfixTransportTable', 'fdPostfixTransportTable',
            array(
              new StringAttribute ('', '', 'fdTransportTableMatch'),
              new CompositeAttribute (
                _('Transport rule'),
                'fdTransportTableRule',
                array(
                  new StringAttribute ('', '', 'transport', TRUE, 'smtp'),
                  new StringAttribute ('', '', 'nexthop', FALSE),
                ),
                '/^([^:]+):(.*)$/',
                '%s:%s', '', ''
              )
            )
          )
        )
      ),
      'section3' => array (
        'name' => _('Restrictions'),
        'attrs' => array (
          new OrderedArrayAttribute (
            new CompositeAttribute (
             _('Restrictions for sender'),
            'postfixSenderRestrictions',
              array(
                new StringAttribute ('', '', 'first2'),
                new SelectAttribute ('', '', 'filter2', TRUE, array('FILTER'), 'FILTER'),
                new StringAttribute ('', '', 'second2'),
              ),
              '/^(.*) ([^:]+) (.*)$/',
              '%s %s %s',
              '', _('For sender')
            )
          ),
          new OrderedArrayAttribute (
            new CompositeAttribute (
             _('Restrictions for recipient'),
            'postfixRecipientRestrictions',
              array(
                new StringAttribute ('', '', 'first3'),
                new SelectAttribute ('', '', 'filter3', TRUE, array('FILTER'), 'FILTER'),
                new StringAttribute ('', '', 'second3'),
              ),
              '/^(.*) ([^:]+) (.*)$/',
              '%s %s %s',
              '', _('For recipient')
            )
          )
        )
      )
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
  }

  function ldap_save ($cleanup = TRUE)
  {
    parent::ldap_save($cleanup);
    $this->attributesAccess['fdPostfixTransportTable']->postLdapSave($this->config->get_ldap_link());
  }
}
?>
