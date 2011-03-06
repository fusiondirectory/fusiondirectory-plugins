{if !$init_successfull}
<br>
<b>{msgPool type=siError}</b><br>
{t}Check if the GOsa support daemon (gosa-si) is running.{/t}&nbsp;
<input type='submit' name='retry_init' value="{t}Retry{/t}">
<br>
<br>
{else}

<!-- GENERIC -->
<h2>{t}License usage{/t}</h2>

{$licenseUses}

<input name='opsiLicenseUsagePosted' value='1' type='hidden'>
{/if}

<p class="seperator">&nbsp;</p>
<div style='width:100%; text-align: right; padding:3px;'>
  <input type='submit' name='save_properties' value='{msgPool type='saveButton'}'>
  &nbsp;
  <input type='submit' name='cancel_properties' value='{msgPool type='cancelButton'}'>
</div>

