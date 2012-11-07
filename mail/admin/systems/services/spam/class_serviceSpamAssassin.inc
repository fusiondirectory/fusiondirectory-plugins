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

class spamFlagsAttribute extends StringAttribute
{
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = "", $acl = "", $regexp = "")
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setVisible(FALSE);
  }

  function setValue ($value)
  {
    parent::setValue($value);
    $this->plugin->attributesInfo['section2']['attrs']['saFlags1']->setValue(preg_match('/B/',$this->value));
    $this->plugin->attributesInfo['section2']['attrs']['saFlags2']->setValue(preg_match('/b/',$this->value));
    $this->plugin->attributesInfo['section2']['attrs']['saFlags3']->setValue(preg_match('/C/',$this->value));
    $this->plugin->attributesInfo['section2']['attrs']['saFlags4']->setValue(preg_match('/R/',$this->value));
    $this->plugin->attributesInfo['section2']['attrs']['saFlags5']->setValue(preg_match('/D/',$this->value));
    $this->plugin->attributesInfo['section2']['attrs']['saFlags6']->setValue(preg_match('/P/',$this->value));
  }

  function getValue()
  {
    return 'Array'.$this->plugin->attributesInfo['section2']['attrs']['saFlags1']->computeLdapValue().
      $this->plugin->attributesInfo['section2']['attrs']['saFlags2']->computeLdapValue().
      $this->plugin->attributesInfo['section2']['attrs']['saFlags3']->computeLdapValue().
      $this->plugin->attributesInfo['section2']['attrs']['saFlags4']->computeLdapValue().
      $this->plugin->attributesInfo['section2']['attrs']['saFlags5']->computeLdapValue().
      $this->plugin->attributesInfo['section2']['attrs']['saFlags6']->computeLdapValue();
  }
}

class gospamserver extends simpleService{

  /* This plugin only writes its objectClass */
  var $objectclasses    = array("goSpamServer");
  var $attributes       = array("saRewriteHeader","saTrustedNetworks","saRequiredScore","saFlags","saRule");

    /* Return plugin informations for acl handling  */

  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Spamassassin service"),
          "plDescription" => _("Spamassassin service")." ("._("Services").")",
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 89,
          "plCategory"    => array("server"),
          "plProvidedAcls"=> parent::generatePlProvidedAcls(self::getAttributesInfo())
          ));
  }

 /*!
  *  \brief The main function : information about attributes
  */
  static function getAttributesInfo ()
  {
    return array (
      'section1' => array (
        'name'  => _('Spam tagging'),
        'icon' => 'images/rightarrow.png',
        'attrs' => array (
          new StringAttribute (
            _('Rewrite header'),
            '',
            'saRewriteHeader'
          ),
          new SelectAttribute (
            _('Required score'),
            _('Select required score to tag mail as spam'),
            'saRequiredScore',
            FALSE,
            array (0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20)
          ),
          new SetAttribute (
            new StringAttribute (
              _('Trusted networks'),
              '',
              'saTrustedNetworks'
            )
          )
        )
      ),
      'section2' => array (
        'name' => _('Flags'),
        'attrs'=> array (
          new BooleanAttribute (
            _('Enable use of bayes filtering'),
            '',
            'saFlags1',
            FALSE,FALSE,'','B',''
          ),
          new BooleanAttribute (
            _('Enable bayes auto learning'),
            '',
            'saFlags2',
            FALSE,FALSE,'','b',''
          ),
          new BooleanAttribute (
            _('Enable RBL checks'),
            '',
            'saFlags3',
            FALSE,FALSE,'','C',''
          ),
          new BooleanAttribute (
            _('Enable use of Razor'),
            '',
            'saFlags4',
            FALSE,FALSE,'','R',''
          ),
          new BooleanAttribute (
            _('Enable use of DDC'),
            '',
            'saFlags5',
            FALSE,FALSE,'','D',''
          ),
          new BooleanAttribute (
            _('Enable use of Pyzor'),
            '',
            'saFlags6',
            FALSE,FALSE,'','P',''
          ),
          new spamFlagsAttribute ("", "", "saFlags")
        )
      ),
      'section3' => array (
        'name' => _('Rules'),
        'attrs' => array (
          new SetAttribute (
            new CompositeAttribute (
              _('Edit spam ruleEdit spam rule'),
              'saRule',
              array (
                new StringAttribute (_('Name'), '', 'ruleName'),
                new TextAreaAttribute (_('Rule'), '', 'ruleDesc'),
              ),
              '%s:%s',
              '%s:%s',
              '', _('Edit spam rule')
            )
          )
        )
      )
    );
  }

  /*!
   * \brief Constructor of the freeradiusAccount
   *        It will build the select groups choices
   */
  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $this->attributesInfo['section2']['attrs']['saFlags1']->setInLdap(FALSE);
    $this->attributesInfo['section2']['attrs']['saFlags2']->setInLdap(FALSE);
    $this->attributesInfo['section2']['attrs']['saFlags3']->setInLdap(FALSE);
    $this->attributesInfo['section2']['attrs']['saFlags4']->setInLdap(FALSE);
    $this->attributesInfo['section2']['attrs']['saFlags5']->setInLdap(FALSE);
    $this->attributesInfo['section2']['attrs']['saFlags6']->setInLdap(FALSE);
  }
}

?>
