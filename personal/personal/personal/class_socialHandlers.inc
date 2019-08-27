<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2014-2019  FusionDirectory

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

class socialHandlerInvalidValueException extends FusionDirectoryException
{
}

class socialHandler
{
  static function listHandlers ()
  {
    global $class_mapping;
    $available = [];
    foreach ($class_mapping as $class => $path) {
      if (preg_match('/^socialHandler_(.+)/', $class, $m)) {
        $available[$m[1]] = $class::getName();
      }
    }
    return $available;
  }

  static function getName ()
  {
    die('Social handler missing getName method');
  }

  protected $baseurl = '';

  /* Returns a link to the social profile page */
  function linkify ($value)
  {
    return $this->link($this->baseurl.'/'.$value, $value);
  }

  protected function link ($link, $text)
  {
    return '<a href="'.$link.'">'.htmlentities($text).'</a>';
  }

  /* Returns the value if ok or throws a socialHandlerInvalidValueException */
  function validate ($value)
  {
    return $value;
  }
}

class socialHandler_facebook extends socialHandler
{
  static function getName ()
  {
    return _('Facebook');
  }

  protected $baseurl = 'https://www.facebook.com';
}

class socialHandler_twitter extends socialHandler
{
  static function getName ()
  {
    return _('Twitter');
  }

  protected $baseurl = 'https://twitter.com';

  function linkify ($value)
  {
    return $this->link($this->baseurl.'/'.$value, '@'.$value);
  }

  function validate ($value)
  {
    return preg_replace('/^@/', '', $value);
  }
}

class socialHandler_diaspora extends socialHandler
{
  static function getName ()
  {
    return _('Diaspora*');
  }

  protected $baseurl = '';

  function linkify ($value)
  {
    return $value;
  }

  function validate ($value)
  {
    if (!tests::is_email($value)) {
      throw new socialHandlerInvalidValueException(_('Diaspora accounts must look like user@pod'));
    }
    return $value;
  }
}

class socialHandler_linkedin extends socialHandler
{
  static function getName ()
  {
    return _('LinkedIn');
  }

  protected $baseurl = 'https://www.linkedin.com/in';
}

class socialHandler_orcid extends socialHandler
{
  static function getName ()
  {
    return _('ORCID');
  }

  protected $baseurl = 'https://orcid.org';

  function linkify ($value)
  {
    return $this->link($this->baseurl.'/'.$value, $value);
  }

  function validate ($value)
  {
    if (!preg_match('/^[0-9]{4}-[0-9]{4}-[0-9]{4}-[0-9]{3}[0-9X]$/', $value)) {
      throw new socialHandlerInvalidValueException(_('ORCID account IDs must look like XXXX-XXXX-XXXX-XXXX where X are digits'));
    }
    if (static::orcidCheckSum($value) != substr($value, -1)) {
      throw new socialHandlerInvalidValueException(_('Incorrect ORCID value, the checksum does not match'));
    }
    return $value;
  }

  public static function orcidCheckSum (string $orcid): string
  {
    /* Remove hyphens, remove last digit, convert to array */
    $baseDigits = str_split(str_replace('-', '', substr($orcid, 0, -1)));
    $sum = 0;
    foreach ($baseDigits as $baseDigit) {
      $sum = ($sum + $baseDigit) * 2;
    }
    $remainder  = $sum % 11;
    $result     = (12 - $remainder) % 11;
    return (($result == 10) ? "X" : (string)$result);
  }
}
