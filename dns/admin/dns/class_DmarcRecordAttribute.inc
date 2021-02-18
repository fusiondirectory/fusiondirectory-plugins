<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2015-2021  FusionDirectory

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

/*
   adkim:  (plain-text; OPTIONAL; default is "r".)  Indicates whether
      strict or relaxed DKIM Identifier Alignment mode is required by
      the Domain Owner.  See Section 3.1.1 for details.  Valid values
      are as follows:

      r: relaxed mode

      s: strict mode

   aspf:  (plain-text; OPTIONAL; default is "r".)  Indicates whether
      strict or relaxed SPF Identifier Alignment mode is required by the
      Domain Owner.  See Section 3.1.2 for details.  Valid values are as
      follows:

      r: relaxed mode

      s: strict mode

   fo:  Failure reporting options (plain-text; OPTIONAL; default is "0")
      Provides requested options for generation of failure reports.
      Report generators MAY choose to adhere to the requested options.
      This tag's content MUST be ignored if a "ruf" tag (below) is not
      also specified.  The value of this tag is a colon-separated list
      of characters that indicate failure reporting options as follows:

      0: Generate a DMARC failure report if all underlying
         authentication mechanisms fail to produce an aligned "pass"
         result.

      1: Generate a DMARC failure report if any underlying
         authentication mechanism produced something other than an
         aligned "pass" result.

      d: Generate a DKIM failure report if the message had a signature
         that failed evaluation, regardless of its alignment.  DKIM-
         specific reporting is described in [AFRF-DKIM].

      s: Generate an SPF failure report if the message failed SPF
         evaluation, regardless of its alignment.  SPF-specific
         reporting is described in [AFRF-SPF].

   p: Requested Mail Receiver policy (plain-text; REQUIRED for policy
      records).  Indicates the policy to be enacted by the Receiver at
      the request of the Domain Owner.  Policy applies to the domain
      queried and to subdomains, unless subdomain policy is explicitly
      described using the "sp" tag.  This tag is mandatory for policy
      records only, but not for third-party reporting records (see
      Section 7.1).  Possible values are as follows:

      none:  The Domain Owner requests no specific action be taken
         regarding delivery of messages.

      quarantine:  The Domain Owner wishes to have email that fails the
         DMARC mechanism check be treated by Mail Receivers as
         suspicious.  Depending on the capabilities of the Mail
         Receiver, this can mean "place into spam folder", "scrutinize
         with additional intensity", and/or "flag as suspicious".

      reject:  The Domain Owner wishes for Mail Receivers to reject
         email that fails the DMARC mechanism check.  Rejection SHOULD
         occur during the SMTP transaction.  See Section 10.3 for some
         discussion of SMTP rejection methods and their implications.

   pct:  (plain-text integer between 0 and 100, inclusive; OPTIONAL;
      default is 100).  Percentage of messages from the Domain Owner's
      mail stream to which the DMARC policy is to be applied.  However,
      this MUST NOT be applied to the DMARC-generated reports, all of
      which must be sent and received unhindered.  The purpose of the
      "pct" tag is to allow Domain Owners to enact a slow rollout
      enforcement of the DMARC mechanism.  The prospect of "all or
      nothing" is recognized as preventing many organizations from
      experimenting with strong authentication-based mechanisms.  See
      Section 6.6.4 for details.  Note that random selection based on
      this percentage, such as the following pseudocode, is adequate:

       if (random mod 100) < pct then
         selected = true
       else
         selected = false

   rf:  Format to be used for message-specific failure reports (colon-
      separated plain-text list of values; OPTIONAL; default is "afrf").
      The value of this tag is a list of one or more report formats as
      requested by the Domain Owner to be used when a message fails both
      [SPF] and [DKIM] tests to report details of the individual
      failure.  The values MUST be present in the registry of reporting
      formats defined in Section 11; a Mail Receiver observing a
      different value SHOULD ignore it or MAY ignore the entire DMARC
      record.  For this version, only "afrf" (the auth-failure report
      type defined in [AFRF]) is presently supported.  See Section 7.3
      for details.  For interoperability, the Authentication Failure
      Reporting Format (AFRF) MUST be supported.

   ri:  Interval requested between aggregate reports (plain-text 32-bit
      unsigned integer; OPTIONAL; default is 86400).  Indicates a
      request to Receivers to generate aggregate reports separated by no
      more than the requested number of seconds.  DMARC implementations
      MUST be able to provide daily reports and SHOULD be able to
      provide hourly reports when requested.  However, anything other
      than a daily report is understood to be accommodated on a best-
      effort basis.

   rua:  Addresses to which aggregate feedback is to be sent (comma-
      separated plain-text list of DMARC URIs; OPTIONAL).  A comma or
      exclamation point that is part of such a DMARC URI MUST be encoded
      per Section 2.1 of [URI] so as to distinguish it from the list
      delimiter or an OPTIONAL size limit.  Section 7.1 discusses
      considerations that apply when the domain name of a URI differs
      from that of the domain advertising the policy.  See Section 12.5
      for additional considerations.  Any valid URI can be specified.  A
      Mail Receiver MUST implement support for a "mailto:" URI, i.e.,
      the ability to send a DMARC report via electronic mail.  If not
      provided, Mail Receivers MUST NOT generate aggregate feedback
      reports.  URIs not supported by Mail Receivers MUST be ignored.
      The aggregate feedback report format is described in Section 7.2.

   ruf:  Addresses to which message-specific failure information is to
      be reported (comma-separated plain-text list of DMARC URIs;
      OPTIONAL).  If present, the Domain Owner is requesting Mail
      Receivers to send detailed failure reports about messages that
      fail the DMARC evaluation in specific ways (see the "fo" tag
      above).  The format of the message to be generated MUST follow the
      format specified for the "rf" tag.  Section 7.1 discusses
      considerations that apply when the domain name of a URI differs
      from that of the domain advertising the policy.  A Mail Receiver
      MUST implement support for a "mailto:" URI, i.e., the ability to
      send a DMARC report via electronic mail.  If not provided, Mail
      Receivers MUST NOT generate failure reports.  See Section 12.5 for
      additional considerations.

   sp:  Requested Mail Receiver policy for all subdomains (plain-text;
      OPTIONAL).  Indicates the policy to be enacted by the Receiver at
      the request of the Domain Owner.  It applies only to subdomains of
      the domain queried and not to the domain itself.  Its syntax is
      identical to that of the "p" tag defined above.  If absent, the
      policy specified by the "p" tag MUST be applied for subdomains.
      Note that "sp" will be ignored for DMARC records published on
      subdomains of Organizational Domains due to the effect of the
      DMARC policy discovery mechanism described in Section 6.6.3.

   v: Version (plain-text; REQUIRED).  Identifies the record retrieved
      as a DMARC record.  It MUST have the value of "DMARC1".  The value
      of this tag MUST match precisely; if it does not or it is absent,
      the entire retrieved record MUST be ignored.  It MUST be the first
      tag in the list.
*/
class DmarcUriListAttribute extends CommaListAttribute
{
  static function dmarcuriencode (string $uri)
  {
    $encoded  = '';
    $length   = mb_strlen($uri);
    for ($i = 0; $i < $length; $i++) {
      $char = mb_substr($uri, $i, 1);
      if (in_array($char, ['%',','], TRUE)) {
        $encoded .= '%'.wordwrap(bin2hex($char), 2, '%', TRUE);
      } else {
        $encoded .= $char;
      }
    }
    return $encoded;
  }

  static function dmarcuridecode (string $uri)
  {
    return rawurldecode($uri);
  }

  function readValues (string $value): array
  {
    return [
      array_map(
        [__CLASS__, 'dmarcuridecode'],
        parent::readValues($value)[0]
      )
    ];
  }

  function writeValues (array $values)
  {
    return parent::writeValues(
      [
        array_map(
          [__CLASS__, 'dmarcuriencode'],
          $values[0]
        )
      ]
    );
  }
}

class DmarcRecordAttribute extends DkimRecordAttribute
{
  public function __construct (string $description, string $ldapName, string $acl = '')
  {
    $attributes = [
      'adkim' => new SelectAttribute(
        _('DKIM Identifier Alignment mode'),
        _('Indicates whether strict or relaxed DKIM Identifier Alignment mode is required by the Domain Owner'),
        $ldapName.'_adkim', FALSE,
        ['','s'], '', [_('Relaxed'), _('Strict')]
      ),
      'aspf' => new SelectAttribute(
        _('SPF Identifier Alignment mode'),
        _('Indicates whether strict or relaxed SPF Identifier Alignment mode is required by the Domain Owner'),
        $ldapName.'_aspf', FALSE,
        ['','s'], '', [_('Relaxed'), _('Strict')]
      ),
      'fo' => new CommaListAttribute(
        $ldapName.'_fo',
        new SetAttribute(
          new SelectAttribute(
            _('Failure reporting options'),
            _('Provides requested options for generation of failure reports'),
            $ldapName.'_fo_value', FALSE,
            ['0','1','d','s']
          )
        ),
        ':'
      ),
      'p' => new SelectAttribute(
        _('Requested Mail Receiver policy'),
        _('Indicates the policy to be enacted by the Receiver at the request of the Domain Owner'),
        $ldapName.'_p', FALSE,
        ['none','quarantine','reject']
      ),
      'sp' => new SelectAttribute(
        _('Subdomain Mail Receiver policy'),
        _('Indicates the policy to be enacted by the Receiver at the request of the Domain Owner'),
        $ldapName.'_sp', FALSE,
        ['none','quarantine','reject']
      ),
      'pct' => new IntAttribute(
        _('Percentage of messages'),
        _('Percentage of messages from the Domain Owner\'s mail stream to which the DMARC policy is to be applied'),
        $ldapName.'_pct', FALSE,
        0, 100
      ),
      /* rf is hidden because only one value is valid */
      'rf' => new HiddenAttribute(
        $ldapName.'_rf', FALSE
      ),
      'ri' => new TimeAttribute(
        _('Report interval'),
        _('Interval requested between aggregate reports'),
        $ldapName.'_ri', FALSE,
        0, FALSE, 86400
      ),
      'rua' => new DmarcUriListAttribute(
        $ldapName.'_rua',
        new SetAttribute(
          new StringAttribute(
            _('Addresses for aggregate feedback'),
            _('Addresses to which aggregate feedback is to be sent'),
            $ldapName.'_rua_uri', FALSE
          )
        )
      ),
      'ruf' => new DmarcUriListAttribute(
        $ldapName.'_ruf',
        new SetAttribute(
          new StringAttribute(
            _('Addresses for failures'),
            _('Addresses to which message-specific failure information is to be reported'),
            $ldapName.'_ruf_uri', FALSE
          )
        )
      ),
    ];
    CompositeAttribute::__construct($description, $ldapName, $attributes, FALSE, FALSE, $acl, NULL);
  }

  function writeValues (array $values)
  {
    $values = array_filter(
      $values,
      function ($value)
      {
        return ($value !== '');
      }
    );
    return implode(
      '; ',
      array_map(
        function ($key, $value) {
          return $key.'='.$value;
        },
        array_keys($values),
        $values
      )
    );
  }

  function check ()
  {
    $error = CompositeAttribute::check();
    if (!empty($error)) {
      return $error;
    }
    return $error;
  }
}
