package FusionInventory::Agent::Config::Ldap;

use strict;
use warnings;

use base qw(FusionInventory::Agent::Config::Backend);

use English qw(-no_match_vars);

sub new {
    my ($class, %params) = @_;

    die "Missing parameter uri" unless $params{uri};
    die "Missing parameter ip"  unless $params{ip};

    my $self = {
        uri => $params{uri},
        ip  => $params{ip}
    };

    if ($param{base}) {
      $self->{base}   = $params{base};
    } else {
      $self->{uri}  =~ m|^(ldap://[^/]+)/([^/]+)$| or die "Missing ldap base";
      $self->{base} = $2;
      $self->{uri}  = $1;
    }

    $self->{bind_dn}   = $params{bind_dn}  if $params{bind_dn};
    $self->{bind_pwd}  = $params{bind_pwd} if $params{bind_pwd};

    bless $self, $class;

    return $self;
}

sub getValues {
    my ($self) = @_;

    my $ldap = Net::LDAP->new( $self->{uri} );
    if ( ! defined $ldap ) {
        warn "LDAP 'new' error: '$@' with uri '".$self->{uri}."'";
        return;
    }

    my $mesg;
    if( defined $self->{bind_dn} ) {
        if( defined $self->{bind_pwd} ) {
            $mesg = $ldap->bind( $self->{bind_dn}, password => $self->{bind_pwd} );
        } else {
            $mesg = $ldap->bind( $self->{bind_dn} );
        }
    } else {
        $mesg = $ldap->bind();
    }

    if ( $mesg->code != 0 ) {
        warn "LDAP bind error: ".$mesg->error." (".$mesg->code.")";
        return;
    }

    my %values;
    my %params =
    {
      'server'                  => 'fiAgentServer',
      'local'                   => 'fiAgentLocal',
      'delaytime'               => 'fiAgentDelaytime',
      'wait'                    => 'fiAgentWait',
      'lazy'                    => 'fiAgentLazy',
      'stdout'                  => 'fiAgentStdout',
      'no-task'                 => 'fiAgentNoTask',
      'scan-homedirs'           => 'fiAgentScanHomedirs',
      'html'                    => 'fiAgentHtml',
      'backend-collect-timeout' => 'fiAgentBackendCollectTimeout',
      'force'                   => 'fiAgentForce',
      'tag'                     => 'fiAgentTag',
      'additional-content'      => 'fiAgentAdditionalContent',
      'no-p2p'                  => 'fiAgentNoP2p',
      'proxy'                   => 'fiAgentProxy',
      'user'                    => 'fiAgentUser',
      'password'                => 'fiAgentPassword',
      'ca-cert-dir'             => 'fiAgentCaCertDir',
      'ca-cert-file'            => 'fiAgentCaCertFile',
      'no-ssl-check'            => 'fiAgentNoSslCheck',
      'timeout'                 => 'fiAgentTimeout',
      'no-httpd'                => 'fiAgentNoHttpd',
      'httpd-ip'                => 'fiAgentHttpdIp',
      'httpd-port'              => 'fiAgentHttpdPort',
      'httpd-trust'             => 'fiAgentHttpdTrust',
      'logger'                  => 'fiAgentLogger',
      'logfile'                 => 'fiAgentLogfile',
      'logfile-maxsize'         => 'fiAgentLogfileMaxsize',
      'logfacility'             => 'fiAgentLogfacility',
      'color'                   => 'fiAgentColor',
      'daemon'                  => 'fiAgentDaemon',
      'no-fork'                 => 'fiAgentNoFork',
      'debug'                   => 'fiAgentDebug',
    };
    @booleans =
    (
      'no-httpd',
      'no-fork',
      'no-p2p',
      'daemon'
    );

    $ldap->cat($self->{dn});
    $mesg = $ldap->search(
        base   => $self->{base},
        filter => "(&(objectClass=fusionInventoryAgent)(ipHostNumber=".$self->{ip}."))",
        attrs => [values(%params)]
    );

    if(scalar($mesg->entries)==1) {
        while (my ($key,$value) = each(%params)) {
            if (($mesg->entries)[0]->exists("$value")) {
                if (grep {$_ eq $key} @booleans) {
                    $values{"$key"} = ($mesg->entries)[0]->get_value("$value") eq "TRUE";
                } else {
                    $values{"$key"} = ($mesg->entries)[0]->get_value("$value");
                }
            } else {
                if (grep {$_ eq $key} @booleans) {
                    $values{"$key"} = 0;
                } else {
                    $values{"$key"} = "";
                }
            }
        }
        return %values;
    } elsif(scalar($mesg->entries)==0) {
        $mesg = $ldap->search( # perform a search
                    base   => $self->{base},
                    filter => "ipHostNumber=".$self->{ip},
                    attrs => [ 'dn' ]
                );
        if (scalar($mesg->entries)>1) {
            warn "Several computers are associated to IP ".$self->{ip}.".";
            return;
        } elsif (scalar($mesg->entries)<1) {
            warn "There is no computer associated to IP ".$self->{ip}.".";
            return;
        }
        my $dn = ($mesg->entries)[0]->dn();
        my $mesg = $ldap->search( # perform a search
            base    => $self->{base},
            filter  => "(&(objectClass=fusionInventoryAgent)(member=$dn))",
            attrs   => [values(%params)]
        );
        if(scalar($mesg->entries)==1) {
            while (my ($key,$value) = each(%params)) {
                if (($mesg->entries)[0]->get_value("$value")) {
                    if (grep {$_ eq $key} @booleans) {
                        $values{"$key"} = ($mesg->entries)[0]->get_value("$value") eq "TRUE";
                    } else {
                        $values{"$key"} = ($mesg->entries)[0]->get_value("$value");
                    }
                } else {
                    if (grep {$_ eq $key} @booleans) {
                        $values{"$key"} = 0;
                    } else {
                        $values{"$key"} = "";
                    }
                }
            }
            return %values;
        } else {
            warn "This computer (".$self->{ip}.") is not configured in LDAP to run this module (missing service fusionInventoryAgent).";
            return;
        }
    } else {
        warn "Several computers are associated to IP ".$self->{ip}.".";
        return;
    }
}

1;
__END__

=head1 NAME

FusionInventory::Agent::Config::LDAP - LDAP-based configuration backend

=head1 DESCRIPTION

This is a LDAP configuration backend.
