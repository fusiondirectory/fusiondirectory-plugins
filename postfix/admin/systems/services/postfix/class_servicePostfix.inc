<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2018  FusionDirectory

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

class MailDomainAttribute extends StringAttribute
{
  /*! \brief Return the ldap value in the correct intern format value
   *
   *  \param $ldapValue The value as found in the LDAP
   */
  function inputValue ($ldapValue)
  {
    /* remove initial '@' from value */
    return preg_replace('/^@/', '', $ldapValue);
  }

  /*! \brief Computes LDAP value
   */
  function computeLdapValue ()
  {
    return '@'.$this->getValue();
  }

  function setValue ($value)
  {
    /* remove initial '@' if needed */
    return parent::setValue(preg_replace('/^@/', '', $value));
  }
}


class servicePostfix extends simpleService
{
  var $objectclasses = array('fdPostfixServer');

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Postfix (SMTP)'),
      'plDescription' => _('Postfix (SMTP)').' ('._('Services').')',
      'plIcon'        => 'geticon.php?context=applications&icon=smtp&size=16',

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  /*!
  *  \brief The main function : information about attributes
  */
  static function getAttributesInfo ()
  {
    return array (
      'section1' => array (
        'name'  => _('Information'),
        'attrs' => array (
          new StringAttribute (
            _('Hostname'), _('The host name.'),
            'postfixMyHostname', TRUE,
            '', '', '/^([^\.]+\.)*[^\.]+$/', 'smtp'
          ),
          new StringAttribute (
            _('Domain'), _('The domain.'),
            'postfixMyDomain', TRUE,
            '', '', '/^([^\.]+\.)*[^\.]+$/', 'example.org'
          ),
          new IntAttribute (
            _('Max mail header size (KB)'), _('This value specifies the maximal header size.'),
            'postfixHeaderSizeLimit', FALSE,
            FALSE, FALSE, 0
          ),
          new IntAttribute (
            _('Max mailbox size (KB)'), _('Defines the maximal size of mail box.'),
            'postfixMailboxSizeLimit', FALSE,
            FALSE, FALSE, 0
          ),
          new IntAttribute (
            _('Max message size (KB)'), _('Specify the maximal size of a message.'),
            'postfixMessageSizeLimit', FALSE,
            FALSE, FALSE, 0
          )
        )
      ),
      'routing' => array (
        'name' => _('Domains and routing'),
        'attrs' => array (
          new SetAttribute (
            new StringAttribute (
              _('Domains to accept mail for'), _('Postfix networks'),
              'postfixMyDestinations'
            )
          ),
          new SubNodesAttribute (
            _('Transport table'), _('Transport table'),
            'fdPostfixTransportTable', 'fdPostfixTransportTable',
            array(
              new StringAttribute ('', '', 'fdTransportTableMatch'),
              new CompositeAttribute (
                _('Transport rule'),
                'fdTransportTableRule',
                array(
                  new StringAttribute ('', '', 'transport', TRUE, 'smtp'),
                  new StringAttribute ('', '', 'nexthop', FALSE),
                ),
                '/^([^:]+):(.*)$/',
                '%s:%s', '', ''
              )
            )
          ),
          new SubNodesAttribute (
            _('Catchall table'), _('Catchall table'),
            'fdPostfixCatchallTable', 'fdPostfixCatchallTable',
            array(
              new MailDomainAttribute (
                _('Domain'), _('Domain concerned by this catchall rule'),
                'fdCatchallTableDomain', TRUE
              ),
              new MailAttribute (
                _('Recipient'), _('Recipient mail address for this catchall rule'),
                'fdCatchallTableRecipient', TRUE
              )
            )
          ),
          new SubNodesAttribute (
            _('Domain alias table'), _('Domain alias table'),
            'fdPostfixDomainAliasTable', 'fdPostfixDomainAliasTable',
            array(
              new MailDomainAttribute (
                _('From'), _('Domain concerned by this alias rule'),
                'fdDomainAliasTableFrom', TRUE
              ),
              new MailDomainAttribute (
                _('To'), _('Recipient domain for this alias rule'),
                'fdDomainAliasTableTo', TRUE
              )
            )
          )
        )
      ),
      'section3' => array (
        'name' => _('Restrictions'),
        'attrs' => array (
          new OrderedArrayAttribute (
            new CompositeAttribute (
             _('Restrictions for sender'),
            'postfixSenderRestrictions',
              array(
                new StringAttribute ('', '', 'first2'),
                new SelectAttribute ('', '', 'filter2', TRUE, array('FILTER'), 'FILTER'),
                new StringAttribute ('', '', 'second2'),
              ),
              '/^(.*) ([^:]+) (.*)$/',
              '%s %s %s',
              '', _('For sender')
            )
          ),
          new OrderedArrayAttribute (
            new CompositeAttribute (
             _('Restrictions for recipient'),
            'postfixRecipientRestrictions',
              array(
                new StringAttribute ('', '', 'first3'),
                new SelectAttribute ('', '', 'filter3', TRUE, array('FILTER'), 'FILTER'),
                new StringAttribute ('', '', 'second3'),
              ),
              '/^(.*) ([^:]+) (.*)$/',
              '%s %s %s',
              '', _('For recipient')
            )
          )
        )
      )
    );
  }

  protected function shouldSave()
  {
    /* We need ldap_save to always run */
    return TRUE;
  }

  function ldap_save ()
  {
    global $config;
    $errors = parent::ldap_save();
    $this->attributesAccess['fdPostfixTransportTable']->postLdapSave($config->get_ldap_link());
    $this->attributesAccess['fdPostfixCatchallTable']->postLdapSave($config->get_ldap_link());
    $this->attributesAccess['fdPostfixDomainAliasTable']->postLdapSave($config->get_ldap_link());
    return $errors;
  }
}
?>
