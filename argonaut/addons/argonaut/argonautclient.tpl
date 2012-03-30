<table summary="">
<tr><td>
    <input type="checkbox" name="activateArgonaut" id="activateArgonaut" {if $activateArgonaut} checked="checked" {/if}
            onClick="javascript:
{if $member_of_ogroup}
                    changeState('inheritArgonaut');
{/if}
                    changeState('argonautClientPort');
                    changeState('argonautWakeOnLanInterface');
                    changeState('argonautTaskIdFile');"/>
    <label for="activateArgonaut">{t}Activate argonaut on this workstation{/t}</label>
  </td>
  <td>
{if $member_of_ogroup}
    <input type="checkbox" name="inheritArgonaut" id="inheritArgonaut" {if $inheritArgonaut} checked="checked" {/if}
            onClick="javascript:
                    changeState('argonautClientPort');
                    changeState('argonautWakeOnLanInterface');
                    changeState('argonautTaskIdFile');"
            {if !$activateArgonaut} disabled="disabled" {/if} />
    <label for="inheritArgonaut">{t}Inherit group informations{/t}</label>
{/if}
  </td>
</tr>
<tr>
  <td>
    <label for="argonautClientPort">{t}Client port:{/t}</label>
  </td><td>
    <input  type="text" name="argonautClientPort" id="argonautClientPort"
            value="{$argonautClientPort}"
            {if !$activateArgonaut or $inheritArgonaut} disabled="disabled" {/if} />
  </td>
</tr>
<tr>
  <td>
    <label for="argonautWakeOnLanInterface">{t}WakeOnLan interface:{/t}</label>
  </td><td>
    <input  type="text" name="argonautWakeOnLanInterface" id="argonautWakeOnLanInterface"
            value="{$argonautWakeOnLanInterface}"
            {if !$activateArgonaut or $inheritArgonaut} disabled="disabled" {/if} />
  </td>
</tr>
<tr>
  <td>
    <label for="argonautTaskIdFile">{t}TaskId file:{/t}</label>
  </td><td>
    <input  type="text" name="argonautTaskIdFile" id="argonautTaskIdFile"
            value="{$argonautTaskIdFile}"
            {if !$activateArgonaut or $inheritArgonaut} disabled="disabled" {/if} />
  </td>
</tr>
<tr>
  <td>
    <label for="argonautLogDir">{t}TaskId file:{/t}</label>
  </td><td>
    <input  type="text" name="argonautLogDir" id="argonautLogDir"
            value="{$argonautLogDir}"
            {if !$activateArgonaut or $inheritArgonaut} disabled="disabled" {/if} />
  </td>
</tr>
</table>
<input type="hidden" name="argonautClientPosted" value="1">
