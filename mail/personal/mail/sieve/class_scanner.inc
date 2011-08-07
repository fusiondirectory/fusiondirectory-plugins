<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class Scanner
{
	function Scanner(&$script)
	{
		$this->_construct($script);
	}

	function _construct(&$script)
	{
		if ($script === null)
		{
			return;
		}

		$this->tokenize($script);
	}

	function setCommentFunc($callback)
	{
		if ($callback == null || is_callable($callback))
		{
			$this->commentFn_ = $callback;
		}
	}

	function tokenize(&$script)
	{
		$pos = 0;
		$line = 1;
		$script_length = mb_strlen($script);

		while ($pos < $script_length)
		{
			foreach ($this->token_match_ as $class => $regex)
			{
				if (preg_match('/^'. $regex .'/', mb_substr($script, $pos), $match))
				{
					$length = mb_strlen($match[0]);

					if ($class != 'whitespace')
					{
						array_push($this->tokens_, array(
							'class' => $class,
							'text'  => chop(mb_substr($script, $pos, $length)),
							'line'  => $line,
						));
					}
					if ($class == 'unknown')
					{
						return;
					}

					$pos += $length;
					$line += mb_substr_count($match[0], "\n");
					break;
				}
			}
		}

		array_push($this->tokens_, array(
			'class' => 'script-end',
			'text'  => 'script-end',
			'line'  => $line,
		));
	}

	function nextTokenIs($class)
	{
		$offset = 0;
		do
		{
			$next = $this->tokens_[$this->tokenPos_ + $offset++]['class'];
		}
		while ($next == 'comment');

		if (is_array($class))
		{
			return in_array($next, $class);
		}
		else if (is_string($class))
		{
			return (strcmp($next, $class) == 0);
		}
		return false;
	}

	function peekNextToken()
	{
		return $this->tokens_[$this->tokenPos_];
	}

	function nextToken()
	{
		$token = $this->tokens_[$this->tokenPos_++];
		while ($token['class'] == 'comment')
		{
			if ($this->commentFn_ != null)
			{
				call_user_func($this->commentFn_, $token);
			}
			$token = $this->tokens_[$this->tokenPos_++];
		}
		return $token;
	}

	function scriptStart()
	{
		return array(
			'class' => 'script-start',
			'text'  => 'script-start',
			'line'  => 1,
		);
	}

	var $commentFn_ = null;
	var $tokenPos_ = 0;
	var $tokens_ = array();
	var $token_match_ = array (
		'left-bracket'   =>  '\[',
		'right-bracket'  =>  '\]',
		'block-start'    =>  '\{',
		'block-end'      =>  '\}',
		'left-parant'    =>  '\(',
		'right-parant'   =>  '\)',
		'comma'          =>  ',',
		'semicolon'      =>  ';',
		'whitespace'     =>  '[ \r\n\t]+',
		'tag'            =>  ':[[:alpha:]_][[:alnum:]_]*(?=\b)',
		'quoted-string'  =>  '"(?:\\[\\"]|[^\x00"])*"',
		'number'         =>  '[[:digit:]]+(?:[KMG])?(?=\b)',
		'comment'        =>  '(?:\/\*(?:[^\*]|\*(?=[^\/]))*\*\/|#[^\r\n]*\r?\n)',
		'multi-line'     =>  'text:[ \t]*(?:#[^\r\n]*)?\r?\n(\.[^\r\n]+\r?\n|[^\.]*\r?\n)*\.\r?\n',
		'identifier'     =>  '[[:alpha:]_][[:alnum:]_]*(?=\b)',
		'unknown token'  =>  '[^ \r\n\t]+'
	);
}

?>
