<div id="{$sectionId}" class="plugin-section">
  <span class="legend">
    {if $sectionIcon}<img src="{$sectionIcon|escape}" alt=""/>{/if}{$section|escape}
  </span>
  <div>
    <table style="border-collapse:collapse; border:1px solid #B0B0B0; width:100%; vertical-align:top; text-align:left;">
      {$attributes.pc_ids}
    </table>
  </div>
</div>
