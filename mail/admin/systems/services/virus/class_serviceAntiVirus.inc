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

class AvFlagsAttribute extends StringAttribute
{
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = "", $acl = "", $regexp = "")
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setVisible(FALSE);
  }

  function setValue ($value)
  {
    parent::setValue($value);
    $this->plugin->attributesInfo['section1']['attrs']['avFlags1']->setValue(preg_match('/D/',$this->value));
    $this->plugin->attributesInfo['section1']['attrs']['avFlags2']->setValue(preg_match('/S/',$this->value));
    $this->plugin->attributesInfo['section2']['attrs']['avFlags3']->setValue(preg_match('/A/',$this->value));
    $this->plugin->attributesInfo['section2']['attrs']['avFlags4']->setValue(preg_match('/E/',$this->value));
  }

  function getValue()
  {
    return $this->plugin->attributesInfo['section1']['attrs']['avFlags1']->computeLdapValue().
      $this->plugin->attributesInfo['section1']['attrs']['avFlags2']->computeLdapValue().
      $this->plugin->attributesInfo['section2']['attrs']['avFlags3']->computeLdapValue().
      $this->plugin->attributesInfo['section2']['attrs']['avFlags4']->computeLdapValue();
  }
}

class govirusserver extends simpleService{

  var $objectclasses    = array("goVirusServer");

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Antivirus service"),
      "plDescription" => _("Antivirus service")." ("._("Services").")",
      "plSelfModify"  => FALSE,
      "plDepends"     => array(),
      "plCategory"    => array("server"),
      "plProvidedAcls"=> parent::generatePlProvidedAcls(self::getAttributesInfo())
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
        'icon' => 'images/rightarrow.png',
        'attrs' => array (
          new StringAttribute (
            _('Database user'),
            _('User name for the database'),
            'avUser',
            TRUE
          ),
          new StringAttribute (
            _('Database mirror'),
            _('Mirror for the database'),
            'avDatabaseMirror',
            FALSE
          ),
          new StringAttribute (
            _('Http proxy URL'),
            _('URL for the proxy'),
            'avHttpProxyURL',
            FALSE
          ),
          new SelectAttribute (
            _('Maximum threads'),
            _('Maximum number of threads'),
            'avMaxThreads',
            FALSE,
            array (1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20),
            5
          ),
          new IntAttribute (
            _('Max directory recursions'),
            _('Max directory recursions'),
            'avMaxDirectoryRecursions',
            FALSE,
            0,
            FALSE,
            4
          ),
           new IntAttribute (
            _('Checks per day'),
            _('Checks per day'),
            'avChecksPerDay',
            FALSE,
            0,
            FALSE,
            12
          ),
          new BooleanAttribute (_('Enable debugging'),_('Enable debugging for the antivirus'),'avFlags1',FALSE,FALSE,'','D',''),
          new BooleanAttribute (_('Enable mail scanning'),_('Enable mail scanning for the antivirus'),'avFlags2',FALSE,FALSE,'','S','')
          )
        ),
     'section2' => array (
      'name' => _('Archive scanning'),
      'attrs' => array (
            new BooleanAttribute (_('Enable scanning of archives'),_('Enable scanning of archives'),'avFlags3',FALSE,FALSE,'','A',''),
            new BooleanAttribute (_('Block encrypted archives'),_('Block encrypted archives'),'avFlags4',FALSE,FALSE,'','E',''),
            new IntAttribute (
              _('Maximum file size'),
              _(' '),
              'avArchiveMaxFileSize',
            FALSE,
            0,
            FALSE,
            4000
          ),
          new IntAttribute (
            _('Maximum recursion'),
            _(' '),
            'avArchiveMaxRecursion',
            FALSE,
            0,
            FALSE,
            5
          ),
          new IntAttribute (
            _('Maximum compression ratio'),
            _(' '),
            'avArchiveMaxCompressionRatio',
            FALSE,
            0,
            FALSE,
            95
          ),
          new AvFlagsAttribute ("","","avFlags")
        )
      )
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $this->attributesInfo['section1']['attrs']['avFlags1']->setInLdap(FALSE);
    $this->attributesInfo['section1']['attrs']['avFlags2']->setInLdap(FALSE);
    $this->attributesInfo['section2']['attrs']['avFlags3']->setInLdap(FALSE);
    $this->attributesInfo['section2']['attrs']['avFlags4']->setInLdap(FALSE);
    $this->attributesInfo['section2']['attrs']['avFlags3']->setManagedAttributes(
      array (
        'erase' => array (
          FALSE => array (
            'section2' => array(
              'avFlags4','avArchiveMaxFileSize','avArchiveMaxRecursion','avArchiveMaxCompressionRatio',
            )
          )
        )
      )
    );
  }
}

?>
