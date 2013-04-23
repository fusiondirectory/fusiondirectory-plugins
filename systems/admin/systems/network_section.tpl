<style type="text/css">
  div.plugin_section#{$sectionId} > div > h2 {literal}{
    display:none;
  }{/literal}
</style>
{if $attributes.networkSettings.DNSenabled}
<div id="{$sectionId}" class="plugin_section fullwidth">
{else}
<div id="{$sectionId}" class="plugin_section">
{/if}
  <span class="legend">
    {$section}
  </span>
  <div>
    {$attributes.networkSettings.display}
  </div>
</div>
