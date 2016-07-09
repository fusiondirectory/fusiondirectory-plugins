<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

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

class serviceIMAP extends simpleService
{
  /* This plugin only writes its objectClass */
  var $objectclasses = array('fdImapServer');

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('IMAP/POP3 generic service'),
      'plDescription' => _('IMAP/POP3').' ('._('Services').')',
      'plIcon'        => 'geticon.php?context=applications&icon=imap&size=16',

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array();
  }

  function execute()
  {
    return '<b>This server runs an IMAP or POP3 server</b>'.parent::execute();
  }

  protected function post_save()
  {
    parent::post_save();
    if (!$this->initially_was_account) {
      mailMethod::resetMailServersCache();
    }
  }

  protected function post_remove ()
  {
    parent::post_remove();
    mailMethod::resetMailServersCache();
  }
}
?>
