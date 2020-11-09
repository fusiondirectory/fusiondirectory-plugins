<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2015-2020  FusionDirectory

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

class SpfRecordAttribute extends OrderedArrayAttribute
{
  public function __construct (string $ldapName, bool $required = FALSE, array $defaultValue = [], string $acl = '')
  {
    $attribute = new StringAttribute(
      _('Terms'), '',
      $ldapName, $required
    );
    parent::__construct($attribute, TRUE, $defaultValue, TRUE);
  }

  public function computeLdapValue ()
  {
    return implode(' ', $this->getValue());
  }

  public function inputValue ($ldapValue)
  {
    if (is_array($ldapValue)) {
      return $ldapValue;
    } else {
      return explode(' ', $ldapValue);
    }
  }

  public function setValue ($value)
  {
    parent::setValue($this->inputValue($value));
  }

  public function check ()
  {
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    } else {
      foreach ($this->value as $value) {
        if (strpos($value, '=') === FALSE) {
          /*
           * ABNF from RFC

           directive        = [ qualifier ] mechanism
           qualifier        = "+" / "-" / "?" / "~"
           mechanism        = ( all / include
                              / a / mx / ptr / ip4 / ip6 / exists )

           all              = "all"
           include          = "include"  ":" domain-spec
           a                = "a"      [ ":" domain-spec ] [ dual-cidr-length ]
           mx               = "mx"     [ ":" domain-spec ] [ dual-cidr-length ]
           ptr              = "ptr"    [ ":" domain-spec ]
           ip4              = "ip4"      ":" ip4-network   [ ip4-cidr-length ]
           ip6              = "ip6"      ":" ip6-network   [ ip6-cidr-length ]
           exists           = "exists"   ":" domain-spec
          */
          if (!preg_match('/^([+-?~])?(all|include|a|mx|ptr|ip4|ip6|exists)(:.+)?$/', $value, $m)) {
            return new SimplePluginCheckError(
              $this,
              sprintf(_('"%s" is not a valid directive for an SPF record'), $value)
            );
          } else {
            switch ($m[2]) {
              case 'all':
                if (!empty($m[3])) {
                  return new SimplePluginCheckError(
                    $this,
                    _('Directive "all" does not accept a value in an SPF record')
                  );
                }
                break;
              case 'exists':
              case 'include':
              case 'ip4':
              case 'ip6':
                if (empty($m[3])) {
                  return new SimplePluginCheckError(
                    $this,
                    sprintf(_('Directive "%s" needs a value in an SPF record'), $m[2])
                  );
                }
                break;
              default:
                break;
            }
          }
        } elseif (!preg_match('/^[[:alpha:]][-_.[:alnum:]]*=/', $value)) {
          /*
           modifier         = redirect / explanation / unknown-modifier
           redirect         = "redirect" "=" domain-spec
           explanation      = "exp" "=" domain-spec
           unknown-modifier = name "=" macro-string
                              ; where name is not any known modifier
           name             = ALPHA *( ALPHA / DIGIT / "-" / "_" / "." )
          */
          return new SimplePluginCheckError(
            $this,
            sprintf(_('"%s" is not a valid modifier for an SPF record'), $value)
          );
        }

      }
    }
  }
}
