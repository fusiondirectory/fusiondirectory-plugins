{if $content}
###FUSIONDIRECTORY
require ["fileinto", "reject", "vacation"];

{/if}
{if $spamfilter}
# Sort mails with higher spam level
if header :contains "X-Spam-Level" "{$spamlevel}" {literal}{{/literal}
  fileinto "{$spambox}";
{literal}}{/literal}
{/if}
{if $sizefilter}
#  Reject mails with bigger size
if size :over {literal}{{/literal}{$maxsize}{literal}}M{{/literal}
        reject text:
{t}Dear sender,

the mail you sent to our mailsystem has been rejected due
to a user configured maximum mail size ({$maxsize} MB).

Either ask the user to remove the sizelimit, or send smaller pieces.

Thank you,
 the mailserver{/t}
.
        ;
        discard;
{literal}}{/literal}

{/if}
{if $vacation}
# Vacation message
vacation :addresses [{$addrlist}]
"{$vacmsg}" ;

{/if}
{if $dropownmail}
# Do not deliver to own mailbox
discard;

{/if}
