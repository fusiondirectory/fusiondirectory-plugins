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


/*!
 * \brief This class contains all the function to manage the policy
 *        for the kolabAccount class
 */
class PolicyAttribute extends CompositeAttribute
{

  function __construct ($description, $ldapName, $attributes, $acl = "", $label = "Composite attribute")
  {
    parent::__construct ($description, $ldapName, $attributes, "", "", $acl, $label);
  }

  protected function loadAttrValue ($attrs)
  {
    if (isset($attrs[$this->getLdapName()]["count"])) {
      $values = array();
      $values[0] = $attrs[$this->getLdapName()][0];
      $values[1] = array();
      for ($i = 1; $i < $attrs[$this->getLdapName()]["count"]; $i++) {
        $values[1][] = $attrs[$this->getLdapName()][$i];
      }
      $this->setValue($values);
    } else {
      $this->resetToDefault();
    }
  }

  function writeValues($values)
  {
    $value = $values[1];
    array_unshift($value, $values[0]);
    return $value;
  }

  function inputValue ($value)
  {
    return $value;
  }

  function readValues($val)
  {
    trigger_error("No one should call readValues");
  }
}

class kolabAccount extends simplePlugin
{

/*
  var $attributes     = array("kolabFreeBusyFuture", "unrestrictedMailSize", "calFBURL","kolabDelegate","kolabInvitationPolicy");
*/
  var $objectclasses  = array("kolabInetOrgPerson");
  var $displayHeader  = TRUE;

  /* Return plugin informations for acl handling  */
  static function plInfo()
  {
    return array(
      "plShortName"     => _("Kolab"),
      "plDescription"   => _("Kolab account settings")."&nbsp;("._("Connectivity addon").")",
      "plSelfModify"    => TRUE,
      "plPriority"      => 20,
      "plCategory"      => array("users"),
      "plObjectType"    => array("connectivity"),
      "plProvidedAcls"  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

 /*!
  *  \brief The main function : information about attributes
  */
  static function getAttributesInfo ()
  {
    return array (
      'section1' => array (
        'name'  => _('Kolab account'),
        'attrs' => array (
          new SetAttribute (
            new MailAttribute (
              _('Delegations'),
              '',
              'kolabDelegate'
            )
          )
        )
      ),
      'section2' => array (
        'name' => _('Mail size'),
        'attrs' => array (
          new BooleanAttribute (
            _('No mail size restriction for this account'),
            '',
            'unrestrictedMailSize'
          )
        )
      ),
      'section3' => array (
        'name' => _('Free Busy information'),
        'attrs' => array (
          new StringAttribute (
            _('URL'),
            '',
            'calFBURL'
          ),
          new IntAttribute (
            _('Future').' ('._('days').')',
            '',
            'kolabFreeBusyFuture',
            FALSE,FALSE,FALSE,60
          )
        )
      ),
      'section4' => array (
        'name' => _('Invitation policy'),
        'attrs' => array (
          new PolicyAttribute (
            _('Invitation policy'),
            'kolabInvitationPolicy',
            array (
              new SelectAttribute (_('Default policy'), _('Default invitation policy'),'anonymous', TRUE,
                array('ACT_MANUAL','ACT_ALWAYS_ACCEPT', 'ACT_ALWAYS_REJECT', 'ACT_REJECT_IF_CONFLICTS', 'ACT_MANUAL_IF_CONFLICTS'),'',
                array(_("Manual"), _("Always accept"),_("Always reject"),_("Reject if conflicts"),_("Manual if conflicts"))),
              new SetAttribute (
                new CompositeAttribute (
                  _('Invitation policy'),
                  'KolabInvitationPolicySpe',
                    array(
                      new MailAttribute ('', _('mail address'),'mailPolicy'),
                      new SelectAttribute ('', _('policy for this mail address'),'policy', TRUE,
                        array('ACT_MANUAL','ACT_ALWAYS_ACCEPT', 'ACT_ALWAYS_REJECT', 'ACT_REJECT_IF_CONFLICTS', 'ACT_MANUAL_IF_CONFLICTS'),'',
                        array(_("Manual"), _("Always accept"),_("Always reject"),_("Reject if conflicts"),_("Manual if conflicts")))
                    ),
                    '%[^:]: %s',
                    '%s: %s',
                    '', _('Invitation policy')
                )
              )
            )
          )
        )
      )
    );
  }

  function execute ()
  {
    if ($this->attributesInfo['section3']['attrs']['calFBURL']->getValue() == NULL) {
      $this->calFBURL= "https://".$this->parent->by_object['mailAccount']->gosaMailServer."/freebusy/".$this->parent->by_object['mailAccount']->mail.".ifb";
      $this->attributesInfo['section3']['attrs']['calFBURL']->setValue($this->calFBURL);
    }
    return parent::execute();
  }

}
?>
