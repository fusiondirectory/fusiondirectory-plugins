<div id="{$sectionId}" class="plugin-section">
  <span class="legend">
    {if $sectionIcon}<img src="{$sectionIcon|escape}" alt=""/>{/if}{$section|escape}
  </span>
  <div>
    <ul>
    {foreach from=$attributes.stats item=stat}
    <li style="list-style-image:url({$stat.img|escape})">
      {$stat.nb|escape} {$stat.name|escape}
    </li>
    {/foreach}
    </ul>
  </div>
</div>
