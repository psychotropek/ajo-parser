<?php

/**
 * @file
 * Contains the FeedsAJOParser class.
 */

class FeedsAJOParser extends FeedsParser {

	/**
	* Implements FeedsParser::parse().
	*/

	public function parse(FeedsSource $source, FeedsFetcherResult $fetcher_result) {

		require DRUPAL_ROOT.'/sites/all/vendor/autoload.php';

		$result = new FeedsParserResult();
		$files=$fetcher_result->file_list;
		$item = array();
		$item['folder_path'] = pathinfo($files[0])['dirname'];
		$item['files_list'] = $files;

		foreach ($files as $file) {
			$path_parts = pathinfo($file);
			$file_type = $path_parts['extension'];

			$table_match = preg_match('/(table)|(Table)/',$path_parts['filename']);
			$figure_match = preg_match('/(figure)|(Figure)|(_f)/',$path_parts['filename']);
			
			switch($file_type) {
				case "doc":
				case "docx":
					try {
						$client = new GuzzleHttp\Client(['base_uri' => 'https://api.docconverter.pro','verify' => false]);
						$res = $client->post('/Token', [ 'form_params' => [ 'grant_type' => 'password', 'username' => 'chris@quillandcode.com', 'password' => 'AJO_d3v070p'] ]);
						$statusCode = $res->getStatusCode();
						if ( $statusCode != 200 )
						{
							echo 'Invalid request status code: ' . $statusCode;
							die();
						}
						$body = (string)$res->getBody();
						$data = json_decode($body);
						$token = $data->access_token;
						// $token can be stored in site cache or database, please check expiry date ($data->expires_in) before using token again (2 weeks expiration time)

						$docconverterTemplate = "AJO test template";
						if ($table_match > 0)
						{
							//todo: need something that won't strip table formatting?
							$docconverterTemplate = "AJO test template";
						}

						$formData = array();
						$formData[] = [ 'name' => 'file_name', 'contents' => fopen($file, 'r')];
						$formData[] = [ 'name' => 'template', 'contents' => $docconverterTemplate];
						$formData[] = [ 'name' => 'returnHtml', 'contents' => 'true'];
						$formData[] = [ 'name' => 'returnData', 'contents' => 'true'];
						
						$response = $client->request('POST', '/api/converter/convertdoc', [
							'headers' => [ 'Authorization' => 'Bearer ' . $token ],
							'multipart' => $formData,
						]);
                    	
						$test = $response->getBody();
						$handle = (string)$test;

						$handle = preg_replace("%(\t|\r|\v|\f)%","",$handle);
						$handle = preg_replace("%<p>(\s)*&#xa0;(\s)*</p>%","",$handle);
						$handle = preg_replace("%<p>(\s)*<strong>&#xa0;</strong>(\s)*</p>%","",$handle);
						$handle = preg_replace("%<p>(\s)*<em>&#xa0;</em>(\s)*</p>%","",$handle);

						if ($table_match > 0) 
						{
							$table_array = array();
							preg_match_all("%^(.|\n)*<table>%",$handle,$above_table,PREG_PATTERN_ORDER);
							$table_array['above_table'] = trim($above_table[0][0]);
							preg_match_all("%(<table>((.|\n*)*)</table>)%",$handle,$table_data,PREG_PATTERN_ORDER);
							//var_dump($table_data);
							//die;
							$table_array['table_data'] = $table_data[0][0];
							preg_match_all("%</table>(.|\n)*$%",$handle,$table_caption,PREG_PATTERN_ORDER);
							$table_array['table_caption'] = trim($table_caption[0][0]);
							$item['tables'][] = $table_array;
							//todo: get index? it's not in table file name
							//preg_match_all("%(table)|(Table)(.*)_%",$path_parts['filename'],$table_index,PREG_PATTERN_ORDER);
							//$item['tables'][$table_index] = $table_array;
						}
						else
						{
							if ($item['title'] == "") {
								preg_match_all("%<h1>\s*(.*)\s*</h1>%",$handle,$title,PREG_PATTERN_ORDER);
								$item['title'] = trim($title[1][0]);
							}

							if ($item['authors'] == "") {
								preg_match_all("%<h6>\s*Authors\s*</h6>\s*<p>\s*(.*)\s*</p>%",$handle,$authors_match,PREG_PATTERN_ORDER);
								$item['authors'] = trim($authors_match[1][0]);
							}
						
							//todo: don't have yet
							preg_match_all("%<h2>\s*Summary\s*</h2>(((?!<h)(.|\n))*)%",$handle,$summary_match,PREG_PATTERN_ORDER);
							$item['summary'] = $summary_match[1][0];
						
							//todo: don't have yet
							preg_match_all("%<h2>\s*Teaser\s*</h2>(((?!<h)(.|\n))*)%",$handle,$teaser_match,PREG_PATTERN_ORDER);
							$item['teaser'] = $teaser_match[1][0];
						
							if ($item['take_home_points'] == "") {
								preg_match_all("%<h2>\s*Take-Home Points\s*</h2>\s*(((?!<h)(.|\n))*)\s*%",$handle,$points_match,PREG_PATTERN_ORDER);
								$item['take_home_points'] = trim($points_match[1][0]);
							}
							if ($item['body'] == "") {
								preg_match_all("@<h2>\s*Body\s*</h2>(.*)<h2>\s*References\s*</h2>@s",$handle,$body_match,PREG_PATTERN_ORDER);
								$item['body'] = $body_match[1][0];
							}
						
							if ($item['references'] == "") {
								preg_match_all("%<h2>\s*References\s*</h2>(.*)\Z%s",$handle,$ref_match,PREG_PATTERN_ORDER);
								$item['references'] = $ref_match[1][0];
							}
						
							if ($item['topics'] == "") {
								preg_match_all("%<h6>\s*Topics\s*</h6>\s*<p>\s*(((?!<h)(.|\n))*)\s*</p>%",$handle,$topics_match,PREG_PATTERN_ORDER);
								$item['topics'] = $topics_match[1][0];
							}
						
							if ($item['sections'] == "") {
								preg_match_all("%<h6>\s*Sections\s*</h6>\s*<p>\s*(((?!<h)(.|\n))*)\s*</p>%",$handle,$sections_match,PREG_PATTERN_ORDER);
								$item['sections'] = trim($sections_match[1][0]);
							}
						
							if ($item['digital_object_id'] == "") {
								preg_match_all("%<h6>\s*Digital Object ID\s*</h6>\s*<p>\s*(((?!<h)(.|\n))*)\s*</p>%",$handle,$doi_match,PREG_PATTERN_ORDER);
								$item['digital_object_id'] = trim($doi_match[1][0]);
							}
						}

					}
					catch (Exception $exc) {
						echo $exc->getMessage();
					}

				break;
				case "png":
				case "jpg":
				case "jpeg":
				//todo: figure_match needs consistent file thing
					if ($path_parts['filename'] == 'medium_image' || $path_parts['filename'] == 'big_image' || $path_parts['filename'] == 'teaser_image') {
						$image_path = $file;
						$item[$path_parts['filename']]['filepath'] = $image_path;
						$image_xml = $this->exif_custom_get_xmp($image_path);
						$item[$path_parts['filename']]['caption'] = $image_xml['XMP:xmpmeta:rdf:description:description:alt:li'];
					}
					else if ($figure_match > 0) 
					{
						$figure_array = array();
						preg_match_all("%(figure)|(Figure)(.*)_%",$path_parts['filename'],$figure_index,PREG_PATTERN_ORDER);
						$image_path = drupal_get_path('module', 'ajo_test').$path_parts['filename'];
						$figure_array['filepath'] = $image_path;
						$image_xml = $this->exif_custom_get_xmp($image_path);
						$figure_array['caption'] = $image_xml['XMP:xmpmeta:rdf:description:description:alt:li'];
						$item['figures'][$figure_index] = $figure_array; 
					}
					else {
						//I haven't the slightest idea as to how to handle this error. 
					}

				break;
			}

		}
		$result->items[] = $item;
		return $result;
	}

	public function exif_custom_xml_recursion($obj, array &$fields, $name) {
		$namespace = $obj->getDocNamespaces(TRUE);
		$namespace[NULL] = NULL;
	  
		$children = array();
		$attributes = array();
	  
		$text = trim((string) $obj);
		if (strlen($text) === 0) {
		  $text = NULL;
		}
	  
		if (strtolower((string) $obj->getName()) == "bag") {
		  // @todo Add support for bags of objects other than just text?
		  $childValues = array();
		  $objChildren = $obj->children("rdf", TRUE);
		  foreach ($objChildren as $child) {
			$childValues[] = trim((string) $child);
		  }
		  if (count($childValues) > 0) {
			$fields[$name] = $childValues;
		  }
		}
		else {
		  $name = $name . ':' . strtolower((string) $obj->getName());
	  
		  // Get info for all namespaces.
		  if (is_object($obj)) {
			foreach ($namespace as $ns => $nsUrl) {
			  // Attributes.
			  $objAttributes = $obj->attributes($ns, TRUE);
			  foreach ($objAttributes as $attributeName => $attributeValue) {
				$attribName = strtolower(trim((string) $attributeName));
				$attribVal = trim((string) $attributeValue);
				if (!empty($ns)) {
				  $attribName = $ns . ':' . $attribName;
				}
				$attributes[$attribName] = $attribVal;
			  }
	  
			  // Children.
			  $objChildren = $obj->children($ns, TRUE);
			  foreach ($objChildren as $childName => $child) {
				$childName = strtolower((string) $childName);
				if (!empty($ns)) {
				  $childName = $ns . ':' . $childName;
				}
				$children[$childName][] = $this->exif_custom_xml_recursion($child, $fields, $name);
			  }
			}
		  }
		  if (!is_null($text)) {
			$fields[$name] = $text;
		  }
		}
	  
		return array(
		  'name' => $name,
		  'text' => html_entity_decode($text),
		  'attributes' => $attributes,
		  'children' => $children,
		);
	}

	public function exif_custom_get_xmp($image) {
		$content = file_get_contents($image);
		$xmp_data_start = strpos($content, '<x:xmpmeta');
		$xmp_data_end = strpos($content, '</x:xmpmeta>');
		if ($xmp_data_start === FALSE || $xmp_data_end === FALSE) {
		  return array();
		}
		$xmp_length = $xmp_data_end - $xmp_data_start;
		$xmp_data = substr($content, $xmp_data_start, $xmp_length + 12);
		unset($content);
		$xmp = simplexml_load_string($xmp_data);
		if ($xmp === FALSE) {
		  return array();
		}
		$field_data = array();
		$this->exif_custom_xml_recursion($xmp, $field_data, 'XMP');
	   
		return $field_data;
	}
	  

	

}
	