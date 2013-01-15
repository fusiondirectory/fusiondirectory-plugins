<?php

class servicePostfix extends simpleService {

  /* This plugin only writes its objectClass */
  var $objectclasses    = array('fdPostfixServer');

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'    => _('Postfix (SMTP)'),
      'plDescription'  => _('Postfix (SMTP)').' ('._('Services').')',
      'plIcon'        => 'plugins/systems/images/service_smtp.png',

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
        'name'  => _('Generic'),
        'icon' => 'images/rightarrow.png',
        'attrs' => array (
          new StringAttribute (
            _('Visible full qualified hostname'), _('The full qualified host name.'),
            'postfixMyHostname', TRUE
          ),
          new StringAttribute (
            _('Visible full qualified domain'), _('The full qualified domain.'),
            'postfixMyDomain', TRUE
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
          ),
          new StringAttribute (
            _('Relay host'), _('Relay messages to following host'),
            'postfixRelayhost'
          ),
          new StringAttribute (
           _('Local networks'), _('Postfix networks'),
           'postfixMyNetworks'
          )
        )
      ),
      'section2' => array (
        'name' => _('Domains and routing'),
        'attrs' => array (
          new SetAttribute (
            new StringAttribute (
            _('Domains to accept mail for'), _('Postfix networks'),
            'postfixMyDestinations'
            )
          ),
          new OrderedArrayAttribute (
            new CompositeAttribute (
             _('Transport'),
            'postfixTransportTable',
              array(
                new StringAttribute ('', '', 'first'),
                new SelectAttribute ('', '', 'filter', TRUE, array('smtp'), '', array('SMTP')),
                new StringAttribute ('', '', 'second'),
              ),
              '%s %[^:]:%s',
              '%s %s:%s',
              '', _('Transport')
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
              '%s %[^:] %s',
              '%s %s %s',
              '', _('Restrictions for sender')
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
              '%s %[^:] %s',
              '%s %s %s',
              '', _('Restrictions for recipient')
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
    $this->attributesAccess['postfixMyDomain']->setDisabled(TRUE);
  }

  function save()
  {
    $this->postfixMyDomain = $this->postfixMyHostname;
    $this->prepare_save();
    $this->ldap_save();
  }
}
?>
