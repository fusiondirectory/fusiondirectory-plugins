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

/* Forces to use a FQDN with final dot */
class FQDNAttribute extends StringAttribute
{
  function validate ()
  {
    $regexp = '/^(([a-z0-9\-]{2,63})\.)+$/';
    $error  = _('"%s" must contain a fully qualified domain name and end with a final dot.<br/><br/>Example: dns1.example.com.');
    if (!preg_match($regexp, $this->value)) {
      return sprintf($error, $this->getLabel());
    }
  }
}

class DnsRecordAttribute extends CompositeAttribute
{
  protected $parentIPs4 = array('');
  protected $parentIPs6 = array('');
  protected $parentFQDN = '';

  public static $types = array(
    'aRecord'     => 'A',
    'a6Record'    => 'A6',
    'aAAARecord'  => 'AAAA',
    'aFSDBRecord' => 'AFSDB',
    'certRecord'  => 'CERT',
    'cNAMERecord' => 'CNAME',
    'dSRecord'    => 'DS',
    'hInfoRecord' => 'HINFO',
    'KeyRecord'   => 'KEY',
    'kXRecord'    => 'KX',
    'LocRecord'   => 'LOC',
    'mDRecord'    => 'MD',
    'mInfoRecord' => 'MINFO',
    'mXRecord'    => 'MX',
    'nAPTRRecord' => 'NAPTR',
    'nSRecord'    => 'NS',
    'nSECRecord'  => 'NSEC',
    'nXTRecord'   => 'NXT',
    'pTRRecord'   => 'PTR',
    'rRSIGRecord' => 'RRSIG',
    'SigRecord'   => 'SIG',
    'sRVRecord'   => 'SRV',
    'sSHFPRecord' => 'SSHFP',
    'tXTRecord'   => 'TXT',
  );

  /* Types which might match reverse zone records */
  public static $reverseTypes = array(
    'aRecord', 'aAAARecord'
  );

  function __construct($label, $description, $ldapName, $required, $acl = '')
  {
    $attributes = array(
      new SelectAttribute(_('Type'), '', $ldapName.'_type', $required, array_keys(self::$types), '', array_values(self::$types)),
      new HiddenAttribute($ldapName.'_nofield'),
      new SelectAttribute(
        _('Reverse zone'), _('Reverse zone this record should be in, if any'),
        'reverse', FALSE
      ),
    );
    parent::__construct($description, $ldapName, $attributes, FALSE, FALSE, $acl, $label);
    $this->setLinearRendering(TRUE);
    $this->attributes[0]->setSubmitForm(TRUE);
    $this->updateFields();
    $this->setRequired($required);
  }

  function setParentIPs($ipv4, $ipv6, $fqdn)
  {
    $this->parentIPs4   = $ipv4;
    $this->parentIPs4[] = '';
    $this->parentIPs6   = $ipv6;
    $this->parentIPs6[] = '';
    $this->parentFQDN   = $fqdn;
    if ($this->attributes[0]->getValue() == 'aRecord') {
      $this->attributes[1]->setDefaultValue(reset($this->parentIPs4));
    } elseif ($this->attributes[0]->getValue() == 'aAAARecord') {
      $this->attributes[1]->setDefaultValue(reset($this->parentIPs6));
    } elseif ($this->attributes[0]->getValue() == 'cNAMERecord') {
      $this->attributes[1]->setDefaultValue(reset($this->parentFQDN));
    }
    if ($this->attributes[1]->getValue() == '') {
      $this->attributes[1]->resetToDefault();
    }
  }

  protected function updateFields()
  {
    $type = $this->attributes[0]->getValue();
    $ldapName = $this->getLdapName().'_'.$type.'_content';
    if ($this->attributes[1]->getLdapName() != $ldapName) {
      switch ($type) {
        case 'aRecord':
          $this->attributes[1] = new IPv4Attribute(
            _('Target'), _('An IPv4 address'),
            $ldapName, TRUE,
            reset($this->parentIPs4)
          );
        break;
        case 'aAAARecord':
          $this->attributes[1] = new IPv6Attribute(
            _('Target'), _('An IPv6 address'),
            $ldapName, TRUE,
            reset($this->parentIPs6)
          );
        break;
        case 'mXRecord':
          $this->attributes[1] = new CharSeparatedCompositeAttribute(
            _('MX Record'),
            $ldapName,
            array(
              new IntAttribute(
                _('Priority'), _('Preference given to this RR among others at the same owner, lower values are preferred'),
                $ldapName.'_priority', TRUE,
                0, FALSE
              ),
              new StringAttribute(
                _('Target'), _('Domain name which specifies a host willing to act as a mail exchange for the owner name'),
                $ldapName.'_target', TRUE
              ),
            ),
            ' '
          );
        break;
        case 'nSRecord':
          $this->attributes[1] = new FQDNAttribute(
            _('Target'), _('Domain name which specifies a host which should be authoritative for the specified class and domain'),
            $ldapName, TRUE
          );
        break;
        case 'sRVRecord':
          $this->attributes[1] = new CharSeparatedCompositeAttribute(
            _('SRV Record'),
            $ldapName,
            array(
              new IntAttribute(
                _('Priority'), _('Priority of the target host, lower value means more preferred'),
                $ldapName.'_priority', TRUE,
                0, FALSE
              ),
              new IntAttribute(
                _('Weight'), _('Relative weight for records with the same priority, higher value means more preferred'),
                $ldapName.'_weight', TRUE,
                0, FALSE
              ),
              new IntAttribute(
                _('Port'), _('TCP or UDP port on which the service is to be found'),
                $ldapName.'_port', TRUE,
                0, FALSE
              ),
              new StringAttribute(
                _('Target'), _('Canonical hostname of the machine providing the service, ending in a dot'),
                $ldapName.'_target', TRUE
              ),
            ),
            ' '
          );
        break;
        case 'cNAMERecord':
          $this->attributes[1] = new StringAttribute(
            _('Redirect to'), _('Domain that this subdomain is an alias of'),
            $ldapName, TRUE,
            $this->parentFQDN
          );
        break;
        default:
          $this->attributes[1] = new StringAttribute(_('Content'), _('Content of this record'), $ldapName, TRUE);
        break;
      }
      if (in_array($type, self::$reverseTypes)) {
        $this->attributes[2]->setVisible(TRUE);
      } else {
        $this->attributes[2]->setVisible(FALSE);
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
    $this->attributes[2]->setValue($values[2]);
  }

  function resetToDefault ()
  {
    $this->attributes[0]->resetToDefault();
    $this->updateFields();
    $this->attributes[1]->resetToDefault();
    $this->attributes[2]->resetToDefault();
  }

  function check ()
  {
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    }
    $type = $this->attributes[0]->getValue();
    if (in_array($type, self::$reverseTypes) && ($this->attributes[2]->getValue() != '')) {
      $ip       = $this->attributes[1]->getValue();
      $reverse  = $this->attributes[2]->getValue();
      $mask     = preg_replace('/\.(in-addr|ip6)\.arpa\.?$/i', '', $reverse);
      if ($type == 'aRecord') {
        $testString = implode('.', array_reverse(explode('.', $ip)));
      } elseif ($type == 'aAAARecord') {
        $testString = implode('.', array_reverse(preg_split('/:|/', expandIPv6($ip), NULL, PREG_SPLIT_NO_EMPTY)));
      }

      if (!preg_match('/\.'.preg_quote($mask).'$/', $testString)) {
        return _('The entered IP does not match the selected reverse zone');
      }
    }
  }
}
