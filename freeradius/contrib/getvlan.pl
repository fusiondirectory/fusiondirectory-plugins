#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
#
#
# Based in the example code of rlm_perl
# Is used to get the vlan from the user radius group from ldap
#

use strict;
use Net::LDAP;
use Net::LDAP::Util;
use Net::LDAP::Message;
use Net::LDAP::Search;

use Argonaut::Common qw(:ldap);

# use ...
# This is very important ! Without this script will not get the filled hashesh from main.
use vars qw(%RAD_REQUEST %RAD_REPLY %RAD_CHECK);
use Data::Dumper;

# This is hash wich hold original request from radius
#my %RAD_REQUEST;
# In this hash you add values that will be returned to NAS.
#my %RAD_REPLY;
#This is for check items
#my %RAD_CHECK;

#LDAP vars

my $configfile = "/etc/argonaut/argonaut.conf";

my $config = Config::IniFiles->new( -file => $configfile, -allowempty => 1, -nocase => 1);

my $ldap_configfile                 =   $config->val( ldap => "config"      ,"/etc/ldap/ldap.conf");
my $ldap_dn                         =   $config->val( ldap => "dn"          ,"");
my $ldap_password                   =   $config->val( ldap => "password"    ,"");

#
# This the remapping of return values
#
use constant    RLM_MODULE_REJECT=>    0;#  /* immediately reject the request */
use constant    RLM_MODULE_FAIL=>      1;#  /* module failed, don't reply */
use constant    RLM_MODULE_OK=>        2;#  /* the module is OK, continue */
use constant    RLM_MODULE_HANDLED=>   3;#  /* the module handled the request, so stop. */
use constant    RLM_MODULE_INVALID=>   4;#  /* the module considers the request invalid. */
use constant    RLM_MODULE_USERLOCK=>  5;#  /* reject the request (user is locked out) */
use constant    RLM_MODULE_NOTFOUND=>  6;#  /* user not found */
use constant    RLM_MODULE_NOOP=>      7;#  /* module succeeded without doing anything */
use constant    RLM_MODULE_UPDATED=>   8;#  /* OK (pairs modified) */
use constant    RLM_MODULE_NUMCODES=>  9;#  /* How many return codes there are */

# Function to handle authorize
sub authorize {
  my $ldapinfos = argonaut_ldap_init ($ldap_configfile, 0, $ldap_dn, 0, $ldap_password);

  if ( $ldapinfos->{'ERROR'} > 0) {
    &radiusd::radlog(1, $ldapinfos->{'ERRORMSG'});
    return RLM_MODULE_FAIL;
  }

  my ($ldap,$LDAPBASE) = ($ldapinfos->{'HANDLE'},$ldapinfos->{'BASE'});

  my $sres = $ldap->search(
    base    => $LDAPBASE,
    filter  => "(&(objectClass=radiusProfile)(memberUid=$RAD_REQUEST{'User-Name'}))",
    scope   => "sub",
    attrs   => ['cn','radiusAuthType','radiusSessionTimeout','radiusIdleTimeout','radiusTunnelType','radiusTunnelMediumType','radiusTunnelPrivateGroupId']
  );

  if ($sres->count == 0) {
    return RLM_MODULE_NOOP;
  }


  my $entry = $sres->entry(0);
  if ($entry->exists('radiusAuthType')) {
    &radiusd::radlog(1, "radiusAuthType: ".$entry->get_value('radiusAuthType'));
  }
  if ($entry->exists('radiusTunnelPrivateGroupId')) {
    &radiusd::radlog(1, "radiusTunnelPrivateGroupId: ".$entry->get_value('radiusTunnelPrivateGroupId'));
  }
  if ($entry->exists('radiusTunnelPrivateGroupId')) {
    $RAD_REPLY{'Tunnel-Private-Group-Id'} = $entry->get_value('radiusTunnelPrivateGroupId');
  }
  if ($entry->exists('cn')) {
    &radiusd::radlog(1, "cn: ".$entry->get_value('cn'));
  }

  $ldap->unbind;

  return RLM_MODULE_OK;
}

sub xlat {
}

sub detach {
  &radiusd::radlog(0,"rlm_perl::Detaching. Reloading. Done.");
}

