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

/*!
 * \brief This class contains all the function to manage the working time
 *        for the proxyAccount class
 */
class workingTime extends CompositeAttribute
{
  /*!
   * \brief Write working time values
   */
  function writeValues($values)
  {
    return $values[0] * 60 + $values[1];
  }

  /*!
   * \brief Read working time values
   */
  function readValues($val)
  {
    $tab = array();
    $val = intval($val);
    $tab[1] = $val % 60;
    $tab[0] = ($val - $tab[1]) / 60;
    return $tab;
  }

}

class proxyAccount extends simplePlugin
{
  /* Definitions */
  var $displayHeader    = TRUE;
  var $objectclasses    = array('gosaProxyAccount');

  /*!
   * \brief  Static  Function returning an ACL information array.
   *
   * \return Array   Returns an ACL array
   */
  static function plInfo()
  {
    return array(
      'plShortName'     => _('Proxy'),
      'plDescription'   => _('This allow to store basic squid settings in ldap for each user'),
      'plFilter'        => '(objectClass=gosaProxyAccount)',
      'plIcon'          => 'geticon.php?context=applications&icon=squid&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=squid&size=16',
      'plSelfModify'    => TRUE,
      'plPriority'      => 9,
      'plObjectType'    => array('user'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array(
      'section1' => array(
        'name'      => _('Proxy account'),
        'template'  => get_template_path('proxyAccount.tpl', TRUE, dirname(__FILE__)),
        'class'     => array('fullwidth'),

        'attrs' => array (
          new BooleanAttribute(
            _('Filter unwanted content (i.e. pornographic or violence related)'),
            _('filterF'),
            'gosaProxyAcctFlags',
            FALSE,
            FALSE
          ),
          new workingTime (
            _('Working Start'),
            'gosaProxyWorkingStart',
            array (
              new SelectAttribute ( '',  _('HourStart'),   'hourstart',   TRUE, array(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24), 1, array('01','02','03','04','05','06','07','08','09','10','11','12','13','14','15','16','17','18','19','20','21','22','23','24')),
              new SelectAttribute ( ':', _('MinuteStart'), 'minutestart', TRUE, array('00','15','30','45'))
            ),
            // read format (not used)
            '',
            // write format (not used)
            '',
            // acl
            '',
            _('Limit proxy access to working time')
          ),
          new workingTime (
            _('Working Stop'),
            'gosaProxyWorkingStop',
            array (
              new SelectAttribute ( _('-'), _('HourStart'), 'hourstop',   TRUE, array(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24), 1, array('01','02','03','04','05','06','07','08','09','10','11','12','13','14','15','16','17','18','19','20','21','22','23','24')),
              new SelectAttribute ( _(':'), _('MinuteStop'), 'minutestop', TRUE, array('00','15','30','45'))
            ),
            '',
            ''
          ),
          new CompositeAttribute (
            _('Restrict proxy usage by quota'),
            'gosaProxyQuota',
            array(
              new IntAttribute    ('',  _('Up'), 'quota_size', FALSE, 0, FALSE, 21),
              new SelectAttribute ('', '', 'quota_unit', FALSE, array('k','m','g'), '', array('KB','MB','GB')),
            ),
            '/^(\\d+)(.)$/',
            '%d%s',
             // acl
            '',
            _('Restrict proxy usage by quota')
          ),
          new SelectAttribute (
            _('per'),
            _('per'),
            'gosaProxyQuotaPeriod',
            FALSE,
            array ('h','d','w','m'), "", array (_("hour"),_("day"),_("week"),_("month"))
          ),
          new BooleanAttribute (
            _('Start Working Hours'),
            _('Starting hours for internet access'),
            'enableWorkingTime',
            FALSE,
            FALSE,
            // acl
            'gosaProxyWorkingStart'
          ),
          new BooleanAttribute (
            _('Proxy Quota'),
            _('Max Data quota for the proxy'),
            'enableQuota',
            FALSE,
            FALSE,
             // acl
            'gosaProxyQuota'
          )
        )
      )
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    $this->attributesInfo['section1']['attrs']['gosaProxyWorkingStart']->setLinearRendering(TRUE);
    $this->attributesInfo['section1']['attrs']['gosaProxyWorkingStop']->setLinearRendering(TRUE);
    $this->attributesInfo['section1']['attrs']['gosaProxyQuota']->setLinearRendering(TRUE);
    $this->attributesInfo['section1']['attrs']['enableWorkingTime']->setValue(isset($this->attrs['gosaProxyWorkingStart']));
    $this->attributesInfo['section1']['attrs']['enableWorkingTime']->setInLdap(FALSE);
    $this->attributesInfo['section1']['attrs']['enableWorkingTime']->setManagedAttributes(
      array(
        'erase' => array(
          FALSE => array(
            'gosaProxyWorkingStart',
            'gosaProxyWorkingStop',
          )
        )
      )
    );
    $this->attributesInfo['section1']['attrs']['enableQuota']->setValue(isset($this->attrs['gosaProxyQuota']));
    $this->attributesInfo['section1']['attrs']['enableQuota']->setInLdap(FALSE);
    $this->attributesInfo['section1']['attrs']['enableQuota']->setManagedAttributes(
      array(
        'erase' => array(
          FALSE => array(
            'gosaProxyQuota',
            'gosaProxyQuotaPeriod',
          )
        )
      )
    );
  }
}
?>
