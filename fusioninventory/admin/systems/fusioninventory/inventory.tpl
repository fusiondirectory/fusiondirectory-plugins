<div id="{$sectionId}" class="plugin_section{$sectionClasses}">
  <span class="legend">
    {$section}
  </span>
  <div>
    {foreach from=$attributes item=infos}
    <ul class="inventory">
      {foreach from=$infos key=cn item=object}
      <li class="{$object.class}">
        <a href="#" onclick="Effect.toggle('toggle_{$cn}', 'blind', {literal}{ duration: 0.3 }{/literal}); return false;">
          {$cn}
        </a>
        <div id="toggle_{$cn}" style="overflow: visible; display: none;">
          <table>
            {foreach from=$object.infos key=label item=value}
              <tr>
                <th>{$label}</th><td>{$value}</td>
              </tr>
            {/foreach}
          </table>
        </div>
      </li>
      {/foreach}
    </ul>
    {/foreach}
  </div>
</div>

