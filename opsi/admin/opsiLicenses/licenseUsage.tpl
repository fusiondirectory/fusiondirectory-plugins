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
