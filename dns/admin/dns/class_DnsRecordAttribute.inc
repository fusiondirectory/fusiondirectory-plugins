<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2015-2016  FusionDirectory

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

class LocRecordAttribute extends CompositeAttribute
{
  function __construct ($description, $ldapName, array $attributes, $acl = '', $label = NULL)
  {
    $readFormat = '/^(?P<lat>[[:digit:] \.]+ [NS])\s+'.
                  '(?P<long>[[:digit:] \.]+ [EW])\s+'.
                  '(?P<alt>[[:digit:]\.]+)m?'.
                  '(\s+(?P<size>[[:digit:]\.]+)m?(\s+(?P<hp>[[:digit:]\.]+)m?(\s+(?P<vp>[[:digit:]\.]+)m?)?)?)?$/';
    parent::__construct ($description, $ldapName, $attributes, $readFormat, FALSE, $acl, $label);
  }

  function writeValues(array $values)
  {
    /* '%s %s %.2fm[ %.2fm[ %.2fm[ %.2fm]]]' */
    $str = sprintf('%s %s %.2fm', $values['lat'], $values['long'], $values['alt']);
    if ($values['size'] != '') {
      $str .= sprintf(' %.2fm', $values['size']);
      if ($values['hp'] != '') {
        $str .= sprintf(' %.2fm', $values['hp']);
        if ($values['vp'] != '') {
          $str .= sprintf(' %.2fm', $values['vp']);
        }
      }
    }
    return $str;
  }
}

class LocRecordLatLongAttribute extends CompositeAttribute
{
  function __construct ($label, $description, $ldapName, array $attributes, $acl = '')
  {
    $readFormat = '/^(?P<degrees>[[:digit:]]+)'.
                  '(\s+(?P<minutes>[[:digit:]]+)(\s+(?P<seconds>[[:digit:]\.]+))?)?'.
                  '\s+(?P<dir>[NSEW])$/';
    $attributes['seconds']->setStep(.001);
    parent::__construct ($description, $ldapName, $attributes, $readFormat, FALSE, $acl, $label);
    $this->setLinearRendering(TRUE);
  }

  function writeValues(array $values)
  {
    /* The format is: d1 [m1 [s1]] {"N"|"S"|"E"|"W"} */
    $str = $values['degrees'];
    if ($values['minutes'] != '') {
      $str .= ' '.$values['minutes'];
      if ($values['seconds'] != '') {
        $str .= ' '.$values['seconds'];
      }
    }
    $str .= ' '.$values['dir'];
    return $str;
  }
}

class DnsRecordAttribute extends CompositeAttribute
{
  protected $parentIPs4 = array('');
  protected $parentIPs6 = array('');
  protected $parentFQDN = '';

  public static $types = array(
    'aRecord'     => 'A',
    //~ 'a6Record'    => 'A6',
    'aAAARecord'  => 'AAAA',
    'aFSDBRecord' => 'AFSDB',
    'certRecord'  => 'CERT',
    'cNAMERecord' => 'CNAME',
    'dSRecord'    => 'DS',
    //~ 'hInfoRecord' => 'HINFO',
    'KeyRecord'   => 'KEY',
    'kXRecord'    => 'KX',
    'LocRecord'   => 'LOC',
    //~ 'mDRecord'    => 'MD',
    //~ 'mInfoRecord' => 'MINFO',
    'mXRecord'    => 'MX',
    'nAPTRRecord' => 'NAPTR',
    'nSRecord'    => 'NS',
    'nSECRecord'  => 'NSEC',
    //~ 'nXTRecord'   => 'NXT',
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

  function __construct($label, $description, $ldapName, $required)
  {
    $attributes = array(
      new SelectAttribute(_('Type'), '', $ldapName.'_type', $required, array_keys(static::$types), '', array_values(static::$types)),
      new HiddenAttribute($ldapName.'_nofield'),
      new SelectAttribute(
        _('Reverse zone'), _('Reverse zone this record should be in, if any'),
        'reverse', FALSE
      ),
    );
    parent::__construct($description, $ldapName, $attributes, FALSE, FALSE, '', $label);
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

  private function loadLocRecordFields($ldapName)
  {
    $this->attributes[1] = new LocRecordAttribute(
      _('LOC Record'),
      $ldapName,
      array(
        'lat' => new LocRecordLatLongAttribute(
          _('Latitude'), _('Latitude'),
          $ldapName.'_latitude',
          array(
            'degrees' => new IntAttribute(
              '', _('Degrees'),
              $ldapName.'_lat_degrees', TRUE,
              0, 90, ''
            ),
            'minutes' => new IntAttribute(
              '°', _('Minutes'),
              $ldapName.'_lat_minutes', FALSE,
              0, 59, ''
            ),
            'seconds' => new FloatAttribute(
              '’', _('Seconds'),
              $ldapName.'_lat_seconds', FALSE,
              0, 59.999, ''
            ),
            'dir' => new SelectAttribute(
              '', _('North/South'),
              $ldapName.'_lat_direction', TRUE,
              array('N', 'S'), '',
              array(_('North'), _('South'))
            ),
          )
        ),
        'long' => new LocRecordLatLongAttribute(
          _('Longitude'), _('Longitude'),
          $ldapName.'_longitude',
          array(
            'degrees' => new IntAttribute(
              '', _('Degrees'),
              $ldapName.'_long_degrees', TRUE,
              0, 180, ''
            ),
            'minutes' => new IntAttribute(
              '°', _('Minutes'),
              $ldapName.'_long_minutes', FALSE,
              0, 59, ''
            ),
            'seconds' => new FloatAttribute(
              '’', _('Seconds'),
              $ldapName.'_long_seconds', FALSE,
              0, 59.999, ''
            ),
            'dir' => new SelectAttribute(
              '', _('East/West'),
              $ldapName.'_long_direction', TRUE,
              array('E', 'W'), '',
              array(_('East'), _('West'))
            ),
          )
        ),
        'alt' => new FloatAttribute(
          _('Altitude (meters)'), '',
          $ldapName.'_altitude', TRUE,
          -100000.00, 42849672.95, ''
        ),
        'size' => new FloatAttribute(
          _('Size (meters)'), '',
          $ldapName.'_size', FALSE,
          0, 90000000, ''
        ),
        'hp' => new FloatAttribute(
          _('Horizontal precision (meters)'), '',
          $ldapName.'_hp', FALSE,
          0, 90000000, ''
        ),
        'vp' => new FloatAttribute(
          _('Vertical precision (meters)'), '',
          $ldapName.'_vp', FALSE,
          0, 90000000, ''
        ),
      )
    );
  }

  private function loadNaptrRecordFields($ldapName)
  {
    $this->attributes[1] = new CharSeparatedCompositeAttribute(
      _('NAPTR Record'),
      $ldapName,
      array(
        new IntAttribute(
          _('Order'), _('Integer specifying the order in which the NAPTR records MUST be processed to ensure the correct ordering of rules.  Low numbers are processed before high numbers.'),
          $ldapName.'_order', TRUE,
          0, FALSE
        ),
        new IntAttribute(
          _('Preference'), _('Integer that specifies the order in which NAPTR records with equal "order" values SHOULD be processed, low numbers being processed before high numbers.'),
          $ldapName.'_pref', TRUE,
          0, FALSE
        ),
        new StringAttribute(
          _('Flags'), _('Flags to control aspects of the rewriting and interpretation of the fields in the record. Flags are single characters from the set [A-Z0-9].  The case of the alphabetic characters is not significant.'),
          $ldapName.'_flags', TRUE
        ),
        new StringAttribute(
          _('Service'), _('Specifies the service(s) available down this rewrite path. It may also specify the particular protocol that is used to talk with a service. A protocol MUST be specified if the flags field states that the NAPTR is terminal.'),
          $ldapName.'_service', TRUE
        ),
        new StringAttribute(
          _('Regular Expression'), _('A STRING containing a substitution expression that is applied to the original string held by the client in order to construct the next domain name to lookup.'),
          $ldapName.'_regexp', TRUE
        ),
        new StringAttribute(
          _('Replacement'), _('The next NAME to query for NAPTR, SRV, or address records depending on the value of the flags field.'),
          $ldapName.'_replace', TRUE
        ),
      ),
      ' '
    );
  }

  private function loadSrvRecordFields($ldapName)
  {
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
        case 'LocRecord':
          $this->loadLocRecordFields($ldapName);
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
        case 'nAPTRRecord':
          $this->loadNaptrRecordFields($ldapName);
          break;
        case 'nSRecord':
          $this->attributes[1] = new FQDNAttribute(
            _('Target'), _('Domain name which specifies a host which should be authoritative for the specified class and domain'),
            $ldapName, TRUE
          );
          break;
        case 'sRVRecord':
          $this->loadSrvRecordFields($ldapName);
          break;
        case 'sSHFPRecord':
          $this->attributes[1] = new StringAttribute(
            _('Record'), _('SSHFP record content. Can be obtained using "ssh-keygen -r some.host.tld", or sshfp command for instance'),
            $ldapName, TRUE
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
      if (in_array($type, static::$reverseTypes)) {
        $this->attributes[2]->setVisible(TRUE);
      } else {
        $this->attributes[2]->setVisible(FALSE);
      }
    }
    // Update ACLs
    $this->setAttributes($this->attributes);
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
    if (in_array($type, static::$reverseTypes) && ($this->attributes[2]->getValue() != '')) {
      $ip       = $this->attributes[1]->getValue();
      $reverse  = $this->attributes[2]->getValue();

      if (!static::matchReverseZone($type, $ip, $reverse)) {
        return _('The entered IP does not match the selected reverse zone');
      }
    }
  }

  static function getReverseZoneInfo($ipv6, $ip, $reverse)
  {
    $mask = preg_replace('/\.(in-addr|ip6)\.arpa\.?$/i', '', $reverse);
    if (preg_match('/^([[:digit:]]+[-\/][[:digit:]]+).([\.[:digit:]]+)$/', $mask, $m)) {
      /* Reverse zone using rfc2317 with - or / */
      $mask = $m[2];
    }
    if ($ipv6) {
      $reversedIp = implode('.', array_reverse(preg_split('/:|/', expandIPv6($ip), NULL, PREG_SPLIT_NO_EMPTY)));
    } else {
      $reversedIp = implode('.', array_reverse(explode('.', $ip)));
    }

    return array($mask, $reversedIp);
  }

  static function matchReverseZone($type, $ip, $reverse)
  {
    list($mask, $testString) = static::getReverseZoneInfo(($type == 'aAAARecord'), $ip, $reverse);

    return preg_match('/\.'.preg_quote($mask).'$/', $testString);
  }

  function getAcl ()
  {
    return $this->getHtmlId().'_'.$this->attributes[0]->getValue();
  }

  function getAclInfo ()
  {
    return FALSE;
  }

  function setAttributes (array $attributes)
  {
    parent::setAttributes($attributes);
    $this->attributes[0]->setAcl($this->getHtmlId());
  }
}
