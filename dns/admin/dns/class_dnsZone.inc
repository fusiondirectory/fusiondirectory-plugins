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
    'mXRecord'    => 'MX',
    'tXTRecord'   => 'TXT',
  );

  function __construct($label, $description, $ldapName, $required, $acl = '')
  {
    $attributes = array(
      new SelectAttribute(_('Type'), '', $ldapName.'_type', $required, array_keys(self::$types), '', array_values(self::$types)),
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
        case 'mXRecord':
          $this->attributes[1] = new CompositeAttribute(
            _('MX Record'),
            $ldapName,
            array(
              new IntAttribute(_('Priority'), '', $ldapName.'_priority', TRUE, 0, FALSE),
              new StringAttribute(_('Target'), '', $ldapName.'_target', TRUE),
            ),
            '/^(\S*) (\S*)$/',
            '%s %s'
          );
        break;
        case 'aRecord':
          $this->attributes[1] = new IPv4Attribute(_('Target'), '', $ldapName, TRUE);
        break;
        default:
          $this->attributes[1] = new StringAttribute(_('Content'), '', $ldapName, TRUE);
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

class DnsRecordPlugin extends simplePlugin
{
  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Record'),
        'attrs' => array(
          new StringAttribute (
            _('Subdomain'), _('Relative subdomain name'),
            'relativeSubdomainName', FALSE
          ),
          new DnsRecordAttribute (
            _('Record'), _('DNS Record'),
            'dnsRecord', TRUE
          ),
        )
      ),
    );
  }

  function __construct (&$config, $dn = NULL, $parent = NULL, $value = NULL)
  {
    parent::__construct($config, $dn, $parent);

    $this->attributesAccess['relativeSubdomainName']->setInLdap(FALSE);
    $this->attributesAccess['dnsRecord']->setInLdap(FALSE);
    $this->attributesAccess['dnsRecord']->setLinearRendering(FALSE);
    if (!empty($value)) {
      $this->relativeSubdomainName = array_shift($value);
      $this->dnsRecord = $value;
    }
  }

  function save()
  {
  }
}

class DnsRecordDialog extends GenericDialog
{
  protected $dialogClass = 'DnsRecordPlugin';

  protected $initialDialogValue = NULL;

  function __construct($simplePlugin, &$attribute, $value = array())
  {
    $this->attribute      = $attribute;
    $this->dialog         = new $this->dialogClass($simplePlugin->config, $simplePlugin->dn, $simplePlugin, $value);
    if (!empty($value)) {
      $this->initialDialogValue = $value;
    }
  }

  function dialog_execute ()
  {
    $this->dialog->save_object();
    return $this->dialog->execute()."\n".
    '<p class="plugbottom">'."\n".
    '  <input type="submit" name="add_finish" value="'.msgPool::addButton().'"/>&nbsp;'."\n".
    '  <input type="submit" name="add_cancel" value="'.msgPool::cancelButton().'"/>'."\n".
    '</p>';
  }

  function handle_finish ()
  {
    $this->dialog->save_object();

    /* Check for errors */
    $checks = $this->dialog->check();
    if (count($checks)) {
      foreach ($checks as $msg) {
        msg_dialog::display(_('Error'), $msg, ERROR_DIALOG);
      }
      return $this->dialog->execute();
    } else {
      /* add new/edited zone */
      $ret = array_merge(array($this->dialog->relativeSubdomainName), $this->dialog->dnsRecord);
      $this->attribute->addValue($ret);
      return FALSE;
    }
  }

  function handle_cancel ()
  {
    if ($this->initialDialogValue !== NULL) {
      $this->attribute->addValue($this->initialDialogValue);
    }
    return FALSE;
  }
}

class DnsRecordsAttribute extends DialogOrderedArrayAttribute
{
  protected $dialogClass = 'DnsRecordDialog';

  protected function getAttributeArrayValue($key, $value)
  {
    return array(
      $value[0],
      DnsRecordAttribute::$types[$value[1]],
      $value[2],
    );
  }

  protected function loadAttrValue ($attrs)
  {
    global $config;

    $this->value = array();
    foreach (array_keys(DnsRecordAttribute::$types) as $type) {
      if (isset($attrs[$type]['count'])) {
        for ($i = 0; $i < $attrs[$type]['count']; $i++) {
          $this->value[] = array('', $type, $attrs[$type][$i]);
        }
      }
    }
    if (isset($attrs['dn'])) {
      $ldap = $config->get_ldap_link();
      $ldap->ls('objectClass=dNSZone', $attrs['dn'], array_merge(array('relativeDomainName'), array_keys(DnsRecordAttribute::$types)));
      while ($subattrs = $ldap->fetch()) {
        for ($i = 0; $i < $subattrs['count']; $i++) {
          $type = $subattrs[$i];
          if ($type == 'relativeDomainName') {
            continue;
          }
          for ($j = 0; $j < $subattrs[$type]['count']; $j++) {
            $this->value[] = array($subattrs['relativeDomainName'][0], $type, $subattrs[$type][$j]);
          }
        }
      }
    }
  }

  function fillLdapValue (&$attrs)
  {
    /* Remove crap made by plugin */
    unset ($attrs[$this->getLdapName()]);
    foreach ($this->value as $line) {
      list ($domain, $type, $content) = $line;
      // Only save root records here
      if (empty($domain)) {
        if (!isset($attrs[$type])) {
          $attrs[$type] = array();
        }
        $attrs[$type][] = $content;
      }
    }
  }

  /* Special LDAP treatment that this attribute does after plugin ldap save */
  function postLdapSave ($ldap)
  {
    /* First delete all old nodes */
    $ldap->ls('objectClass=dNSZone', $this->plugin->dn, array('dn'));
    $delete = array();
    while ($attrs = $ldap->fetch()) {
      $delete[] = $attrs['dn'];
    }
    foreach ($delete as $dn) {
      $ldap->rmdir($dn);
    }
    /* Then add our values */
    $nodes = array();
    foreach ($this->value as $line) {
      list ($domain, $type, $content) = $line;
      if (empty($domain)) {
        /* ignore root records */
        continue;
      }
      if (!isset($nodes[$domain])) {
        $nodes[$domain] = array(
          'objectClass'         => array('dNSZone'),
          'relativeDomainName'  => $domain,
          'zoneName'            => $this->plugin->zoneName,
        );
      }
      if (!isset($nodes[$domain][$type])) {
        $nodes[$domain][$type] = array();
      }
      $nodes[$domain][$type][] = $content;
    }
    foreach ($nodes as $node) {
      $dn = 'relativeDomainName='.$node['relativeDomainName'].','.$this->plugin->dn;
      $ldap->cd($dn);
      $ldap->add($node);
      if (!$ldap->success()) {
        msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $dn, LDAP_ADD, get_class()), LDAP_ERROR);
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
        'filter'      => '(&(objectClass=dNSZone)(relativeDomainName=@))',
        'ou'          => 'ou=dns,',
        'icon'        => 'geticon.php?context=applications&icon=dns&size=16',
        'mainAttr'    => 'zoneName'
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
          new HiddenAttribute (
            //~ _('Relative domain name'), _('Relative domain name'),
            'relativeDomainName', TRUE,
            '@'
          ),
        )
      ),
      'records' => array(
        'name'  => _('Records'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new DnsRecordsAttribute(
            '', _('The DNS records for this zone'),
            'dnsRecords', FALSE
          )
        )
      )
    );
  }

  function ldap_save ($cleanup = TRUE)
  {
    parent::ldap_save($cleanup);
    $this->attributesAccess['dnsRecords']->postLdapSave($this->config->get_ldap_link());
  }
}
