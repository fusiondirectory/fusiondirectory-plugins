<div id="{$sectionId}" class="plugin_section{$sectionClasses}">
  <span class="legend">
    {$section}
  </span>
  <div>
    {foreach from=$attributes item=infos}
    <ul>
      {foreach from=$infos key=cn item=object}
      <li style="list-style-image:url(plugins/systems/images/select_server.png)" class="{$object.class}">
        {$cn}
        <ul>
          {foreach from=$object.infos key=label item=value}
            <li style="list-style:disc">
              {$label}:&nbsp;{$value}
            </li>
          {/foreach}
        </ul>
      </li>
      {/foreach}
    </ul>
    {/foreach}
  </div>
</div>
