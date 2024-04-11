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
    return '<a href="'.$link.'">'.htmlescape($text).'</a>';
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

class socialHandler_github extends socialHandler
{
  static function getName ()
  {
    return _('GitHub');
  }

  protected $baseurl = 'https://github.com';

  function validate ($value)
  {
    /* Regex from https://github.com/shinnn/github-username-regex */
    if (!preg_match('/^[a-z\d](?:[a-z\d]|-(?=[a-z\d])){0,38}$/i', $value)) {
      throw new socialHandlerInvalidValueException(_('GitHub account names have at most 39 characters, contain only alphanumeric characters or hyphens, cannot have multiple consecutive hyphens, and cannot begin or end with a hyphen'));
    }
    return $value;
  }
}

class socialHandler_googlescholar extends socialHandler
{
  static function getName ()
  {
    return _('Google Scholar');
  }

  protected $baseurl = 'https://scholar.google.com/citations?user=';

  function linkify ($value)
  {
    return $this->link($this->baseurl.$value, $value);
  }

  function validate ($value)
  {
    /* Regex from https://www.wikidata.org/wiki/Property:P1960 */
    if (!preg_match('/^[-\w]{12}$/', $value)) {
      throw new socialHandlerInvalidValueException(_('Google Scholar account IDs have exactly 12 alphanumerical characters or hyphens or underscores'));
    }
    return $value;
  }
}

class socialHandler_matrix extends socialHandler
{
  static function getName ()
  {
    return _('Matrix');
  }

  protected $baseurl = '';

  function linkify ($value)
  {
    return $value;
  }

  function validate ($value)
  {
    /* Grammar specified in https://matrix.org/docs/spec/appendices#identifier-grammar
     * Currently, we do not check the server name (valid DNS or IPv4 address or IPv6 address) and optional port number
     */
    if (!preg_match('/^@[-=\.\/\w]+:.+$/', $value)) {
      throw new socialHandlerInvalidValueException(_('Matrix user IDs have the format \'@username:server\''));
    }
    return $value;
  }
}
