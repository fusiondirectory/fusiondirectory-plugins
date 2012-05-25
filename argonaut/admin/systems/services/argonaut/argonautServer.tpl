<br>
<h2>{t}Argonaut server{/t}</h2>
<br>
{render acl=$argonautACL}
<table>
<tr>
  <td><label for="argonautDeleteFinished">{t}Delete finished tasks{/t}</label></td>
  <td><input id="argonautDeleteFinished" name = "argonautDeleteFinished" type="checkbox"
   {if $argonautDeleteFinished==1} checked="checked"{/if} /></td>
</tr><tr>
  <td><label for="argonautProtocol">{t}Protocol :{/t}</label></td>
  <td>
    <select id="argonautProtocol" name = "argonautProtocol">
      {html_options values=$protocols output=$protocols selected=$argonautProtocol}
    </select>
  </td>
</tr><tr>
  <td><label for="argonautPort">{t}Port :{/t}</label></td>
  <td><input id="argonautPort" name = "argonautPort" type="number" value="{$argonautPort}" /></td>
</tr><tr>
  <td><label for="argonautWakeOnLanInterface">{t}Interface for WakeOnLan :{/t}</label></td>
  <td><input id="argonautWakeOnLanInterface" name = "argonautWakeOnLanInterface" type="text" value="{$argonautWakeOnLanInterface}" /></td>
</tr><tr>
  <td><label for="argonautIpTool">{t}IP tool :{/t}</label></td>
  <td><input id="argonautIpTool" name = "argonautIpTool" type="text" value="{$argonautIpTool}" /></td>
</tr><tr>
  <td><label for="argonautLogDir">{t}Log directory :{/t}</label></td>
  <td><input id="argonautLogDir" name = "argonautLogDir" type="text" value="{$argonautLogDir}" /></td>
</tr>
</table>
{/render}

<p class="seperator">&nbsp;</p>
<div style="width:100%; text-align:right;padding-top:10px;padding-bottom:3px;">
  <input type='submit' name='SaveService' value='{msgPool type=saveButton}'>
  &nbsp;
  <input type='submit' name='CancelService' value='{msgPool type=cancelButton}'>
</div>
<input type="hidden" name="argonautServerPosted" value="1">
