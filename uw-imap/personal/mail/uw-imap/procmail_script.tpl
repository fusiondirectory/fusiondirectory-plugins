#*** Procmail script for _{$user}_ ***

DROPPRIVS=true


# Include variable definitions from procmail library
INCLUDERC=$PMSRC_/pm-javar.rc


# Check for empty body and discard
:0 B:
*$ ! $NSPC
/dev/null

_{if $spamfilter}_
# procmail spam handling
GOSA_SPAM="yes"

:0
* ! ^From:.*(_{$addrlist}_)
* ! FROM_DAEMON
{

  #   Do not add extra headers. This saves external shell call
  #   (formail). Also do not try to kill the message content,
  #   again saving one external call (awk). With these, the
  #   recipe is faster and more CPU friendly.
  PM_JA_UBE_HDR                  = ""
  JA_UBE_ATTACHMENT_ILLEGAL_KILL = "no"
  JA_UBE_FLAG_FROM_NSLOOKUP      = "no"

  INCLUDERC = $PMSRC/pm-jaube.rc

  #   Variable "ERROR" is set if message was UBE, record error
  #   to log file with "()/"

  :0 :
  * ERROR ?? ()/[a-z].*
  {
    #  Don't save those *.exe, *.zip UBE attachements
    :0
    *  ERROR ?? Attachment-FileIllegal
    /dev/null

    :0 :
    _{$spambox}_
  }
}
_{/if}_
_{if $vacation}_
# Vacation message
GOSA_VACATION=yes
VACATION_CACHE=$HOME/.vacation.cache

:0
*   ^TO__{$mail}_
* ! ^FROM_DAEMON
* ! ^X-Loop: (_{$addrlist}_)
* ! ^Precedence:.*(bulk|list|junk)
{
  :0 c: $HOME/.vacation.$LOCKEXT
  * ! ? formail -rD 8192 $VACATION_CACHE
  {
    # Compose reply and add some basic headers
    :0 fhw
    |   formail -rt                                                     \
        -A "Precedence: junk"                                           \
        -A "X-Loop: _{$mail}_"

    :0 a    # Formail succeeded
    {
      #  Change subject
      :0 fhw
      * ^Subject: */[^ ].*
      | formail -I "Subject: vacation (was: $MATCH)"

      :0 fb           # put message to body
      | echo -n "_{$vacmsg}_"

      :0              # Send it
      | $SENDMAIL
    }
  }
}
_{/if}_
_{if $discard}_
# Discard any local messages
:0 :
/dev/null

_{/if}_
# Care for cleanups

# Remove stale vacation cache
:0
* !  $GOSA_VACATION ?? yes
* ! ? test -f $VACATION_CACHE
{
  rm -f $VACATION_CACHE
}
