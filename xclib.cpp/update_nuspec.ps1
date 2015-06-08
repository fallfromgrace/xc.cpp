param([string]$project)

$date_now = get-date
$date_rev_start = get-date -year 2000 -month 1 -day 1 -hour 0 -minute 0 -second 0
$date_build_start = get-date -year $date_now.year -month $date_now.month -day $date_now.day -hour 0 -minute 0 -second 0

$revision = [int]((New-TimeSpan -start $date_rev_start -end $date_now).TotalDays)
$build = [int]((New-TimeSpan -start $date_build_start -end $date_now).TotalSeconds / 2)

$path = $project + ".nuspec"
$xml = [xml](get-content $path)
$version = $xml.package.metadata.version.Split(".")
$version[2] = $revision
$version[3] = $build
$xml.package.metadata.version = [string]::Join(".", $version)
$xml.package.metadata.id = $project
$xml.package.metadata.title = $project
$xml.Save($path)