<div id="{$sectionId}" class="plugin_section{$sectionClasses}">
  <span class="legend">
    {$section}
  </span>
  <div>
    {foreach from=$attributes item=infos}
    <ul class="inventory">
      {foreach from=$infos key=class item=objects}
        <li class="{$class}">
          <a href="#" onclick="Effect.toggle('toggle_{$class}', 'blind', {literal}{ duration: 0.3 }{/literal}); return false;">
            {$class}
          </a>
          <div id="toggle_{$class}" style="overflow: visible; display: none;">
            <table>
              {foreach from=$objects key=cn item=object}
                <tr>
                    {foreach from=$object key=label item=value}
                      <th>{$label}</th>
                    {/foreach}
                </tr>
              {/foreach}
              {foreach from=$objects key=cn item=object}
                <tr>
                    {foreach from=$object key=label item=value}
                      <td>{$value}</td>
                    {/foreach}
                </tr>
              {/foreach}
            </table>
          </div>
      {/foreach}
    </ul>
    {/foreach}
  </div>
</div>

