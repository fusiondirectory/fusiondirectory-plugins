<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2015  FusionDirectory

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

class serviceSpamAssassin extends simpleService {

  /* This plugin only writes its objectClass */
  var $objectclasses = array('goSpamServer');

    /* Return plugin informations for acl handling  */

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Spamassassin service'),
      'plDescription' => _('Spamassassin service').' ('._('Services').')',
      'plIcon'        => 'geticon.php?context=applications&icon=spamassassin&size=16',

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
        'name'  => _('Spam tagging'),
        'icon' => 'images/rightarrow.png',
        'attrs' => array (
          new StringAttribute (
            _('Rewrite header'), _('Rewrite header'),
            'saRewriteHeader'
          ),
          new SelectAttribute (
            _('Required score'), _('Select required score to tag mail as spam'),
            'saRequiredScore', FALSE,
            array (0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20)
          ),
          new SetAttribute (
            new StringAttribute (
              _('Trusted networks'), _('Trusted networks'),
              'saTrustedNetworks'
            )
          )
        )
      ),
      'section2' => array (
        'name'  => _('Flags'),
        'attrs' => array (
          new BooleanAttribute (
            _('Enable use of bayes filtering'), '',
            'flag_enable_bayes', FALSE,
            FALSE, '', 'B', ''
          ),
          new BooleanAttribute (
            _('Enable bayes auto learning'), '',
            'flag_bayes_autolearn', FALSE,
            FALSE, '', 'b', ''
          ),
          new BooleanAttribute (
            _('Enable RBL checks'), '',
            'flag_enable_rblcheck', FALSE,
            FALSE, '', 'C', ''
          ),
          new BooleanAttribute (
            _('Enable use of Razor'), '',
            'flag_enable_razor', FALSE,
            FALSE, '', 'R', ''
          ),
          new BooleanAttribute (
            _('Enable use of DDC'), '',
            'flag_enable_ddc', FALSE,
            FALSE, '', 'D', ''
          ),
          new BooleanAttribute (
            _('Enable use of Pyzor'), '',
            'flag_enable_pyzor', FALSE,
            FALSE, '', 'P', ''
          ),
          new FlagsAttribute ('saFlags', array(
            'flag_enable_bayes',
            'flag_bayes_autolearn',
            'flag_enable_rblcheck',
            'flag_enable_razor',
            'flag_enable_ddc',
            'flag_enable_pyzor'
          ))
        )
      ),
      'section3' => array (
        'name' => _('Rules'),
        'attrs' => array (
          new SetAttribute (
            new CompositeAttribute (
              _('Edit spam rule'),
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
  }
}

?>
