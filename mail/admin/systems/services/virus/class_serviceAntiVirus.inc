<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

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

class govirusserver extends simpleService {

  var $objectclasses    = array('goVirusServer');

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Antivirus service'),
      'plDescription' => _('Antivirus service').' ('._('Services').')',

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
        'name'  => _('Generic virus filtering'),
        'icon'  => 'images/rightarrow.png',
        'attrs' => array (
          new StringAttribute (
            _('Database user'), _('User name for the database'),
            'avUser', TRUE
          ),
          new StringAttribute (
            _('Database mirror'), _('Mirror for the database'),
            'avDatabaseMirror', FALSE
          ),
          new StringAttribute (
            _('Http proxy URL'), _('URL for the proxy'),
            'avHttpProxyURL', FALSE
          ),
          new SelectAttribute (
            _('Maximum threads'), _('Maximum number of threads'),
            'avMaxThreads', FALSE,
            array (1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20),
            5
          ),
          new IntAttribute (
            _('Max directory recursions'), _('Max directory recursions'),
            'avMaxDirectoryRecursions', FALSE,
            0, FALSE, 4
          ),
           new IntAttribute (
            _('Checks per day'), _('Checks per day'),
            'avChecksPerDay', FALSE,
            0, FALSE, 12
          ),
          new BooleanAttribute (
            _('Enable debugging'), _('Enable debugging for the antivirus'),
            'flag_enable_debug', FALSE,
            FALSE, '', 'D', ''
          ),
          new BooleanAttribute (
            _('Enable mail scanning'), _('Enable mail scanning for the antivirus'),
            'flag_enable_mailscan', FALSE,
            FALSE, '', 'S', ''
          )
        )
      ),
      'section2' => array (
        'name' => _('Archive scanning'),
        'attrs' => array (
          new BooleanAttribute (
            _('Enable scanning of archives'), _('Enable scanning of archives'),
            'flag_enable_archivescan', FALSE,
            FALSE, '', 'A', ''
          ),
          new BooleanAttribute (
            _('Block encrypted archives'), _('Block encrypted archives'),
            'flag_block_encrypted', FALSE,
            FALSE, '', 'E', ''
          ),
          new IntAttribute (
            _('Maximum file size'), _('Maximum file size'),
            'avArchiveMaxFileSize', FALSE,
            0, FALSE, 4000
          ),
          new IntAttribute (
            _('Maximum recursion'), _('Maximum recursion'),
            'avArchiveMaxRecursion', FALSE,
            0, FALSE, 5
          ),
          new IntAttribute (
            _('Maximum compression ratio'), _(' '),
            'avArchiveMaxCompressionRatio', FALSE,
            0, FALSE, 95
          ),
          new FlagsAttribute ('avFlags', array(
            'flag_enable_debug', 'flag_enable_mailscan',
            'flag_enable_archivescan', 'flag_block_encrypted'
          ))
        )
      )
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $this->attributesAccess['flag_enable_archivescan']->setManagedAttributes(
      array (
        'erase' => array (
          FALSE => array (
            'flag_block_encrypted','avArchiveMaxFileSize','avArchiveMaxRecursion','avArchiveMaxCompressionRatio',
          )
        )
      )
    );
  }
}

?>
