<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2015  FusionDirectory

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

class DnsRecordAttribute extends CompositeAttribute
{
  public static $types = array(
    'aRecord'     => 'A',
    'aAAARecord'  => 'AAAA',
    'tXTRecord'   => 'TXT',
  );

  function __construct($label, $description, $ldapName, $required, $acl = '')
  {
    $attributes = array(
      new SelectAttribute('', '', $ldapName.'_type', $required, array_keys(self::$types), '', array_values(self::$types)),
      new SelectAttribute('', '', $ldapName.'_content', $required)
    );
    parent::__construct($description, $ldapName, $attributes, FALSE, FALSE, $acl, $label);
    $this->setLinearRendering(TRUE);
    $this->attributes[0]->setSubmitForm(TRUE);
    $this->updateFields();
    $this->setRequired($required);
  }

  protected function updateFields()
  {
    $type = $this->attributes[0]->getValue();
    $ldapName = $this->getLdapName().'_'.$type.'_content';
    if ($this->attributes[1]->getLdapName() != $ldapName) {
      switch ($type) {
        /* we may have to use a composite attribute for this case
        case 'mXRecord':
          $this->attributes[1] = new IntAttribute('', '', $this->getLdapName().'_int', TRUE);
          $this->attributes[2] = new StringAttribute('', '', $this->getLdapName().'_content', TRUE);
          array_splice($this->attributes, 3);
        break;*/
        case 'aRecord':
          $this->attributes[1] = new IPv4Attribute('', '', $ldapName, TRUE);
        break;
        default:
          $this->attributes[1] = new StringAttribute('', '', $ldapName, TRUE);
        break;
      }
    }
    $this->setAttributes($this->attributes); // Update ACLs
  }

  function applyPostValue()
  {
    parent::applyPostValue();
    $this->updateFields();
  }

  function setValue ($values)
  {
    if (!is_array($values)) {
      $values = $this->inputValue($values);
    }
    $this->attributes[0]->setValue($values[0]);
    $this->updateFields();
    $this->attributes[1]->setValue($values[1]);
  }

  function resetToDefault ()
  {
    $this->attributes[0]->resetToDefault();
    $this->updateFields();
    $this->attributes[1]->resetToDefault();
  }
}

class DnsRecordsAttribute extends OrderedArrayAttribute
{
  function __construct ($label, $description, $ldapName, $required, $values = array(), $edit_enabled = TRUE)
  {
    $attribute = new DnsRecordAttribute(
      $label, $description,
      $ldapName, $required
    );
    parent::__construct($attribute, FALSE, $values, $edit_enabled);
  }

  protected function loadAttrValue ($attrs)
  {
    $this->value = array();
    foreach (array_keys(DnsRecordAttribute::$types) as $type) {
      if (isset($attrs[$type]['count'])) {
        for ($i = 0; $i < $attrs[$type]['count']; $i++) {
          $this->value[] = array($type, $attrs[$type][$i]);
        }
      }
    }
    if ($this->order) {
      $this->reIndexValues();
    }
  }

  function fillLdapValue (&$attrs)
  {
    /* Remove crap made by plugin */
    unset ($attrs[$this->getLdapName()]);

    if ($this->isInLdap()) {
      foreach (array_keys(DnsRecordAttribute::$types) as $type) {
        $value = array();
        foreach ($this->value as $a) {
          if ($a[0] == $type) {
            $value[] = $a[1];
          }
        }
        $attrs[$type] = $value;
      }
    }
  }
}

class dnsZone extends simplePlugin
{
  var $mainTab = TRUE;

  var $objectclasses = array('dNSZone');

  public static function plInfo()
  {
    return array(
      'plShortName'   => _('DNS zone'),
      'plDescription' => _('DNS zone'),
      'plObjectType'  => array('dnsZone' => array(
        'name'        => _('DNS zone'),
        'filter'      => 'objectClass=dNSZone',
        'ou'          => 'ou=dns,',
        'icon'        => 'geticon.php?context=applications&icon=dns&size=16',
        'mainAttr'    => 'relativeDomainName'
      )),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Zone'),
        'attrs' => array(
          new BaseSelectorAttribute('ou=dns,'),
          new HostNameAttribute (
            _('Zone name'), _('Zone name'),
            'zoneName', TRUE
          ),
          new StringAttribute (
            _('Relative domain name'), _('Relative domain name'),
            'relativeDomainName', TRUE,
            '@'
          ),
        )
      ),
      'records' => array(
        'name'  => _('Records'),
        'attrs' => array(
          new DnsRecordsAttribute(
            _('DNS records'), _('The DNS records for this zone'),
            'dnsRecords', FALSE
          )
        )
      )
    );
  }
}
