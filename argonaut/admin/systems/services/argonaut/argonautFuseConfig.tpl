<br/>
<table>
<tr><td colspan="2"><h3>Argonaut fuse config:</h3></td></tr>
{foreach from=$attributes key=key item=value name=loop}
<tr>
  <td><label for="{$key}">
  {if $key=='argonautFuseDefaultMode'}
    {t}Default mode{/t}
  {elseif $key=='argonautFuseLogDir'}
    {t}Log directory{/t}
  {elseif $key=='argonautFusePxelinuxCfg'}
    {t}Pxelinux cfg path{/t}
  {elseif $key=='argonautFusePxelinuxCfgStatic'}
    {t}Pxelinux cfg static path{/t}
  {elseif $key=='argonautFuseFaiFlags'}
    {t}Fai flags{/t}
  {elseif $key=='argonautFuseNfsRoot'}
    {t}NFS root{/t}
  {elseif $key=='argonautFuseOpsiAdmin'}
    {t}Opsi admin{/t}
  {elseif $key=='argonautFuseOpsiPassword'}
    {t}Opsi password{/t}
  {elseif $key=='argonautFuseOpsiServer'}
    {t}Opsi server{/t}
  {elseif $key=='argonautFuseOpsiLang'}
    {t}Opsi lang{/t}
  {elseif $key=='argonautFuseLtspServer'}
    {t}LTSP server{/t}
  {/if}
  </label></td>
  <td><input id="{$key}" name="{$key}" type="text" value="{$value}"/></td>
</tr>
{/foreach}
</table>

<p class="seperator">&nbsp;</p>
<div style="width:100%; text-align:right;padding-top:10px;padding-bottom:3px;">
  <input type='submit' name='SaveService' value='{msgPool type=saveButton}'>
  &nbsp;
  <input type='submit' name='CancelService' value='{msgPool type=cancelButton}'>
</div>
<input type="hidden" name="argonautFuseConfigPosted" value="1">
