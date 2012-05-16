<table summary="">
<tr><td>
    <input type="checkbox" name="activateArgonaut" id="activateArgonaut"
    {if (($activateArgonaut && !$member_of_ogroup) || (!$activateArgonaut && $member_of_ogroup))} checked="checked" {/if}
            onClick="javascript:
                    changeState('argonautClientPort');
                    changeState('argonautClientWakeOnLanInterface');
                    changeState('argonautClientLogDir');
                    changeState('argonautTaskIdFile');
{foreach from=$argonautServiceName key=nameFD item=namePC}
                    changeState('service_{$nameFD}');
{/foreach}"/>
{if $member_of_ogroup}
    <label for="activateArgonaut">{t}Inherit group informations{/t}</label>
{else}
    <label for="activateArgonaut">{t}Activate argonaut on this workstation{/t}</label>
{/if}
  </td>
  <td>
  </td>
</tr>
<tr>
  <td>
    <label for="argonautClientPort">{t}Client port:{/t}</label>
  </td><td>
    <input  type="text" name="argonautClientPort" id="argonautClientPort"
            value="{$argonautClientPort}"
            {if !$activateArgonaut } disabled="disabled" {/if} />
  </td>
</tr>
<tr>
  <td>
    <label for="argonautClientWakeOnLanInterface">{t}WakeOnLan interface:{/t}</label>
  </td><td>
    <input  type="text" name="argonautClientWakeOnLanInterface" id="argonautClientWakeOnLanInterface"
            value="{$argonautClientWakeOnLanInterface}"
            {if !$activateArgonaut } disabled="disabled" {/if} />
  </td>
</tr>
<tr>
  <td>
    <label for="argonautTaskIdFile">{t}TaskId file:{/t}</label>
  </td><td>
    <input  type="text" name="argonautTaskIdFile" id="argonautTaskIdFile"
            value="{$argonautTaskIdFile}"
            {if !$activateArgonaut } disabled="disabled" {/if} />
  </td>
</tr>
<tr>
  <td>
    <label for="argonautClientLogDir">{t}Log directory:{/t}</label>
  </td><td>
    <input  type="text" name="argonautClientLogDir" id="argonautClientLogDir"
            value="{$argonautClientLogDir}"
            {if !$activateArgonaut } disabled="disabled" {/if} />
  </td>
</tr>
<tr>
  <td colspan="2">
    {t}Service names:{/t}
  </td>
</tr>
{foreach from=$argonautServiceName key=nameFD item=namePC}
<tr>
  <td>
    <label for="service_{$nameFD}">{$nameFD}</label>
  </td><td>
    <input  type="text" name="service_{$nameFD}" id="service_{$nameFD}"
            value="{$namePC}"
            {if !$activateArgonaut } disabled="disabled" {/if} />
  </td>
</tr>
{/foreach}
</table>
<input type="hidden" name="argonautClientPosted" value="1">
