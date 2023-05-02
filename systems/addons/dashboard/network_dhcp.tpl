<div id="{$sectionId}" class="plugin-section">
  <span class="legend">
    {if !empty($sectionIcon)}<img src="{$sectionIcon|escape}" alt=""/>{/if}{$section|escape}
  </span>
  <div>
    {foreach from=$attributes item=infos}
    <ul>
    {foreach from=$infos item=server}
    <li style="list-style-image:url(geticon.php?context=devices&amp;icon=server&amp;size=16)" id="server_{$server.name|escape}">
      {$server.link}
      <ul>
        {foreach from=$server.zones item=zone}
        <li style="list-style:disc">
          {if isset($zone.text)}
            {$zone.text|escape}
          {else}
            {$zone.link}
          {/if}
        </li>
        {/foreach}
      </ul>
    </li>
    {/foreach}
    </ul>
    {/foreach}
  </div>
</div>
